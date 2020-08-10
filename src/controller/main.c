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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include "fade.h"
#include "light.h"
#include "pwm.h"
#include "uart.h"


/**
 * Fade the light.
 *
 * This function is responsible for fading the light into its desired state over
 * a given time. If a new fade cycle is initiated, it stores the current light
 * configuration, calculates the new one and fades smothly to it.
 *
 *
 * @param set If `true`, a new fade cycle is initiated. Only @ref parse_command
 *            should use this functionality.
 */
static void
fade_light(bool set)
{
    /* Static variables will be used to store the previous and next colors on
     * initialization and maintain a counter for the current step of the fading
     * process. */
    static uint8_t step = 0;
    static rgb prev, next;

    /* If a new fade cycle should be initialized, store the currently configured
     * color and the one to fade to in the internal variables and reset the
     * counter to its maximum. If there's an ongoing fade cycle, this operation
     * will abort the running one and copy its current state, so it will be
     * continued in the fade cycle now starting. */
    if (set) {
        prev = pwm_get_rgb();
        next = light_rgb();
        step = FADE_STEPS;
    }

    /* If there are remaining steps for an ongoing fading cycle, calculate the
     * color for the current fading step before setting the related PWM output
     * channel registers. */
    if (step > 0) {
        float fade = (FADE_STEPS - (--step)) / FADE_STEPS;
        pwm_set_rgb(prev.r + ((next.r - prev.r) * fade),
                    prev.g + ((next.g - prev.g) * fade),
                    prev.b + ((next.b - prev.b) * fade));

        /* Wait for a slight amount of time before eventually getting to the
         * next fading step to imitate a smooth change between colors. */
        _delay_ms(FADE_WAIT);
    }
}


/**
 * Check if the buffer contains a specific @p command.
 *
 * This function checks, whether the string in @p buffer has the given @p
 * command as its prefix, to check if this command should be handled right now.
 * Additional characters of the @p buffer will not be checked, as they may be
 * parameters of the command. Its intended use is in if-clauses in the @ref
 * parse_command function.
 *
 *
 * @param command The command to be checked.
 *
 * @return Returns true, if @p buffer is a type of @p command, otherwise false.
 */
static inline bool
is_command(const char *buffer, const char *command)
{
    return (strncmp(command, buffer, strlen(command)) == 0);
}


/**
 * Parse a command with integer argument.
 *
 * This function is used in conjunction with @ref parse_command to check the
 * buffer for commands with integer arguments and can be used to avoid redundant
 * code fragments. It will check for the getter and setter commands and execute
 * the apropriate steps if necessary.
 *
 *
 * @param buffer  The buffer to be checked.
 * @param cmd_set The setter command.
 * @param cmd_get The getter command.
 * @param max     The maximum value of the integer argument.
 * @param dst     Where to store the parsed argument.
 *
 * @return If a getter or setter command has been found and executed, true will
 *         be returned, otherwise false.
 */
static bool
parse_command_int(const char *buffer,
                  const char *cmd_set,
                  const char *cmd_get,
                  int max,
                  int *dst)
{
    /* If the buffer contains a setter command, evaluate the argument and write
     * the parsed integer into the destination variable.
     *
     * NOTE: If the parsed integer is not within the specified bounds, this
     *       command will be ignored entirely. */
    if (is_command(buffer, cmd_set)) {
        int tmp = atoi(buffer + strlen(cmd_set) + 1);
        if (tmp >= 0 && tmp <= max)
            *dst = tmp;
        return true;
    }

    /* If the buffer contains a getter command, prepare a send string containing
     * the parameter name and value before sending it via UART back to the
     * client.
     *
     * NOTE: The getter uses the parameter value currently configured, even if
     *       the light is still fading to this color. This allows the user to
     *       see the light's desired state instead of a random snapshot during
     *       fadeing. */
    else if (is_command(buffer, cmd_get)) {
        char tmp[UART_BUFFER_SIZE];
        snprintf(tmp, UART_BUFFER_SIZE, "%s %d\n", cmd_set, *dst);
        uart_send(tmp);
        return true;
    }

    return false;
}


/**
 * No operation.
 *
 * This function has no specific purpose and wont execute any commands. It's
 * intended to be used in @ref parse_command to beautify one-line if clauses.
 * Usually, the compiler will optimize this call, so there is no additional
 * overhead when running the program.
 */
static void
noop()
{}


/**
 * Parse the next command.
 *
 * This function parses the command in a string received via UART, if new data
 * is available, and executes necessary steps for this command.
 */
static void
parse_command()
{
    /* Copy the received command line string into a local buffer, so the next
     * one can be received by the Interrupt Service Routine. If no data is
     * available, the function will return immediately. */
    char buffer[UART_BUFFER_SIZE];
    if (!uart_receive(buffer, UART_BUFFER_SIZE))
        return;


    /****************
     * parse commands
     ****************/

    /* Parse the power status. Although this could be handled by a simple on/off
     * parameter, an integer will be used, so the same code as for the other
     * parameters can be reused not just by the LED controller's firmware, but
     * also the related homebridge plugin or any other third-party code. */
    if (parse_command_int(buffer, "pwr", "?pwr", 1, &(light.power)))
        noop();

    /* Parse additional commands for hue, saturation and luminance with integer
     * parameters. Each of them will have an allowed range from 0 to 100, except
     * for hue, which has a limit of 360. */
    else if (parse_command_int(buffer, "val", "?val", 100, &(light.value)))
        noop();
#if 1
    else if (parse_command_int(buffer, "hue", "?hue", 360, &(light.hue)))
        noop();
    else if (parse_command_int(buffer, "sat", "?sat", 100, &(light.saturation)))
        noop();
#endif


    /* If the parsed command was a setter command, initiate a new fading cycle
     * to fade to the altered color settings. For ongoing fading cycles, the
     * counter will be reset by this operation, but altered data is not lost. */
    if (buffer[0] != '?')
        fade_light(true);
}


int
main()
{
    /* Initialize all components of the firmware and setup all required ports
     * and registers for service. */
    pwm_init();  /* enable PWM */
    uart_init(); /* enable UART */
    sei();       /* enable interupts */

    /* Use an infinite loop for repeatedly update the PWM registers (i.e. to
     * fade colors) and check for new commands to be parsed executed. */
    while (true) {
        /* Check if there's an ongoing fading cycle and update the PWM registers
         * for setting the color of the current fading step. This function also
         * controls the sleep time between two fading steps, if necessary. */
        fade_light(false);

        /* Finally, check the UART buffers for any new commands. If a new
         * command is available, it will be parsed and necessary steps executed
         * by the following function. */
        parse_command();
    }

    return 0;
}
