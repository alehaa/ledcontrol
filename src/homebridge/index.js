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

        this.Characteristic = this.api.hap.Characteristic;


        /* Initialize an empty command queue and start a related worker-function
         * in the background to issue these commands. For further information
         * see the sendCommands() method. */
        this.queue = [];
        setInterval(() => this.sendCommands(), 500);

        /* Cache the current power state to avoid glitches in the homekit UI
         * when returning an offline state until the response from the LED
         * controller has been parsed.
         *
         * TODO: Not just the power state, but all characteristics should be
         *       cached appropriately in an array. */
        this.power = false;
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
        this.service.getCharacteristic(this.Characteristic.On)
            .on('get', this.getPowerState.bind(this))
            .on('set', this.setPowerState.bind(this));

        this.service.getCharacteristic(this.Characteristic.Brightness)
            .on('get', this.getCharacteristic.bind(this, 'val'))
            .on('set', this.setCharacteristic.bind(this, 'val'));

        if (true) {
            this.service.getCharacteristic(this.Characteristic.Hue)
                .on('get', this.getCharacteristic.bind(this, 'hue'))
                .on('set', this.setCharacteristic.bind(this, 'hue'));

            this.service.getCharacteristic(this.Characteristic.Saturation)
                .on('get', this.getCharacteristic.bind(this, 'sat'))
                .on('set', this.setCharacteristic.bind(this, 'sat'));
        }

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
        /* For power responses, the received result will be translated to an
         * intever value, which is cached for subsequent homekit requests. */
        if (response.indexOf('pwr') > -1) {
            this.power = (response == 'pwr on\n') ? 1 : 0;
            this.service.updateCharacteristic(this.Characteristic.On,
                                              this.power);
            return;
        }

        /* Any other responses will have integer values. Therefore, the received
         * value is parsed as such and can be used to handle the characteristics
         * covered below. */
        var value = parseInt(response.split(' ')[1]);
        if (response.indexOf('val') > -1)
            this.service.updateCharacteristic(this.Characteristic.Brightness,
                                              value);

        else if (true) {
            if (response.indexOf('hue') > -1)
                this.service.updateCharacteristic(this.Characteristic.Hue,
                                                  value);

            else if (response.indexOf('sat') > -1)
                this.service.updateCharacteristic(
                    this.Characteristic.Saturation, value);
        }
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

        /* NOTE: This callback needs always to be called, even if no data is
         *       returned. Otheriwse apple devices will asume the device is not
         *       responding. */
        callback(null);
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


    /**
     * Get the current power state.
     *
     * This method issues a get-command to receive the current power state of
     * the connected light.
     *
     * @note This method is nearly identical to @ref getCharacteristic, but adds
     *       returning a cached power state to the callback to improve the user
     *       experience.
     *
     *
     * @param callback The callback to call after issuing the command.
     */
    getPowerState(callback)
    {
        this.queue.push('?pwr');

        /* Pass the cached power state to the callback to avoid ugly glitches in
         * the UI for temporarily showing offline devices until the command
         * issued above is executed. After a restart of homebridge, this result
         * may be inappropriate, as there is no cached state yet, but this
         * should be fine for these rare cases.
         *
         * NOTE: This callback needs always to be called, even if no power state
         *       has been cached yet. Otheriwse apple devices will asume the
         *       device is not responding.*/
        callback(null, this.power);
    }


    /**
     * Set the power state.
     *
     * This method issues a new command to set the power state of the connected
     * light.
     *
     * @note This method is a specialized version of @ref setCharacteristic, as
     *       setting the power state requires a string instead of an integer.
     *
     *
     * @param value    The new power state as integer. Anything other than zero
     *                 will be interpreted as `on`, like for any other boolean.
     * @param callback The callback to call after issuing the command.
     */
    setPowerState(value, callback)
    {
        this.queue.push(value ? 'pwr on' : 'pwr off');

        /* NOTE: This callback needs always to be called, even if no data is
         *       returned. Otheriwse apple devices will asume the device is not
         *       responding. */
        callback(null);
    }
}
