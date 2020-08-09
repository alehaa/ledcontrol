/* This file is part of ledcontrol.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Copyright (C)
 *  2016-2020 Alexander Haase <ahaase@alexhaase.de>
 */

'use strict';

/**
 * Local Serial-IO instance.
 *
 * As the name suggests, this third-party module allows serial communication. As
 * the arduino nano, on which the LED controller is based on, has an inbuild USB
 * to serial converter, it will be used for communication between the host
 * (homebridge) and the LED controller.
 */
const serialio = require('serial-io');


/* Register the module and associate the accessory type name with the related
 * class, which will parse its configuration and handle its requests from
 * homebridge and homekit. */
module.exports = (api) => {
    api.registerAccessory('ArduinoLED', ArduinoLED);
};


/**
 * LED controller homebridge plugin class.
 *
 * This class is a proxy between homebridge and the LED controller and forwards
 * any commands issued by homebridge directly to it, while passing the response
 * back to homebridge. There's no special logic implemented in this plugin, as
 * all of the processing takes place on the LED controller itself.
 */
class ArduinoLED
{
    /**
     * Constructor.
     *
     *
     * @param log    The logging instance.
     * @param config The configuration for the light to be controlled.
     * @param api    The homebridge API.
     */
    constructor(log, config, api)
    {
        this.log = log;
        this.config = config;
        this.api = api;


        /* Gather a list of all characteristics supported of the light depending
         * on the configuration passed to this plugin instance. It will be
         * statically cached, as it doesn't change over runtime. */
        this.characteristics = this.getSupportedCharacteristics();

        /* Initialize an empty command queue and start a related worker-function
         * in the background to issue these commands. For further information
         * see the sendCommands() method. */
        this.queue = [];
        setInterval(() => this.sendCommands(), 500);

        /* Initialize an empty cache for previously received responses from the
         * LED controller. These will be used as response for commands, before
         * the (relative slow) UART response is received, parsed and send to the
         * homekit client. */
        this.cache = [];
    }


    /**
     * Get the supported characteristics for this light.
     *
     * This method gathers all characteristics supported by this light depending
     * on the configuration passed to this plugin instance.
     *
     *
     * @return The supported characteristics for this light.
     */
    getSupportedCharacteristics()
    {
        const characteristic = this.api.hap.Characteristic;
        const tmp = {
            'pwr' : characteristic.On,
            'val' : characteristic.Brightness
        };

        if (true) {
            tmp.hue = characteristic.Hue;
            tmp.sat = characteristic.Saturation;
        }

        return tmp;
    }


    /**
     * Get the supported services for this light.
     *
     * This method returns a list of supported services for this light, which is
     * (you guessed it) a lightbulb service.
     *
     *
     * @return A list of services supported by this light.
     */
    getServices()
    {
        /* Create a new lightbulb service. The name of the controlled light will
         * be passed as argument to create a unique lightbulb fixture in
         * homebridge and homekit. */
        this.service = new this.api.hap.Service.Lightbulb(this.config.name);

        /* Create handlers for the specific characteristics and map them to the
         * internal methods of this class. */
        for (const [command, char] of Object.entries(this.characteristics))
            this.service.getCharacteristic(char)
                .on('get', this.getCharacteristic.bind(this, command))
                .on('set', this.setCharacteristic.bind(this, command));

        /* Return the list of services, this class instance provides for the
         * controlled light. For this plugin, this is just the lightbulb
         * service configured above. */
        return [ this.service ];
    }


    /**
     * Send the commands from the command queue.
     *
     * This method runs in the background to communicate with the LED controller
     * via UART. An extra method is required, as multiple commands may be
     * received nearly at the same time by homebridge, while UART communication
     * can handle a single command simultaniously only.
     */
    sendCommands()
    {
        /* Foreach command in the command queue, send this specific command to
         * the LED controller and await the reponse. The Serial-IO wrapper takes
         * care on handling the low-level communication via UART. */
        this.queue.forEach((cmd, index) => {
            serialio
                .send(this.config.adapter, cmd + '\n',
                      {baudRate : this.config.baudrate || 9600})

                /* After sucessfully receiveing a response via UART, forward the
                 * response to the handler method to parse it. Empty responses
                 * from set-commands will be ignored, as homekit doesn't await
                 * any data from these commands. */
                .then(response => {
                    if (response)
                        this.handleResponse(response);
                    this.queue.splice(index, 1);
                })

                /* If an error occured, forward the error message to homebridge.
                 * In this case the command will not be removed from the queue,
                 * so it can be repeated by the next call of this method. */
                .catch(error => this.log.error(error));
        });
    }


    /**
     * Handle a response from the LED controller.
     *
     * This method parses the given @p response and updates the characteristics
     * of the homebridge lightbulb service with the data received.
     *
     *
     * @param response The response to handle.
     */
    handleResponse(response)
    {
        /* Check if the related command is a valid response to be handled and if
         * so, start processing the response. */
        const command = response.split(' ')[0];
        if (command in this.characteristics) {
            /* All responses handled by this method will have integer values as
             * paramter. Therefore, the received value is parsed as such and can
             * be used to handle the characteristics covered below. */
            const value = parseInt(response.split(' ')[1]);

            /* Cache the obtained response value for later replies by the
             * getCharacteristic method before updating the related
             * characteristic in homebridge / homekit. */
            this.cache[command] = value;
            this.service.updateCharacteristic(this.characteristics[command],
                                              value);
        }

        /* If the command can't be handled by this plugin, issue an error
         * message in the homebridge logs. */
        else
            this.log.error('unable to handle response for command ' + command);
    }


    /**
     * Get the current state of a specific @p characteristic.
     *
     * This method issues a get-command to receive the current state of the
     * given @p characteristic for the connected light.
     *
     *
     * @param characteristic The characteristic to lookup.
     * @param callback       The callback to call after issuing the command.
     */
    getCharacteristic(characteristic, callback)
    {
        this.queue.push('?' + characteristic);

        /* Pass the cached state to the callback to avoid ugly glitches in the
         * homekit UI for temporarily showing no data for the device until the
         * command issued above is executed. After a restart of homebridge, this
         * result may be inappropriate, as there is no cached state yet, but
         * this should be fine for these rare cases.
         *
         * NOTE: This callback needs always to be called, even if no data is
         *       returned. Otheriwse apple devices will asume the device is not
         *       responding. */
        callback(null, this.cache[characteristic] || 0);
    }


    /**
     * Set a new @p value for a specific @p characteristic.
     *
     * This method issues a set-command to update the @p value of the given @p
     * characteristic for the connected light.
     *
     *
     * @param value    The new value to be set.
     * @param callback The callback to call after issuing the command.
     */
    setCharacteristic(characteristic, value, callback)
    {
        this.queue.push(characteristic + ' ' + value);

        /* NOTE: This callback needs always to be called, even if no data is
         *       returned. Otheriwse apple devices will asume the device is not
         *       responding. */
        callback(null);
    }
}
