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

#include "light.h"
#include "uart.h"


/**
 * Check if the buffer contains a specific @p command.
 *
 * This macro checks, whether the string in `buffer` has the given @p command as
 * its prefix, to check if this command should be handled right now. Additional
 * characters of the `buffer` will not be checked, as they may be parameters of
 * the command. Its intended use is in if-clauses in the @ref parse_command
 * function.
 *
 *
 * @param command The command to be checked.
 *
 * @return Returns true, if `buffer` is a type of @p command, otherwise false.
 */
#define IS_COMMAND(command) (strncmp(command, buffer, strlen(command)) == 0)


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
    if (IS_COMMAND(cmd_set)) {
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
    else if (IS_COMMAND(cmd_get)) {
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

    /* Set the power status either to 'on' or 'off' to enable or disable the
     * light. To simplify the implementation, any other parameter value than
     * 'on' will be evaluated as 'off'. */
    if (IS_COMMAND("pwr"))
        light.power = IS_COMMAND("pwr on");

    /* Get the current power status. */
    else if (IS_COMMAND("?pwr"))
        uart_send(light.power ? "pwr on\n" : "pwr off\n");

    /* Parse additional commands for hue, saturation and luminance with integer
     * parameters. Each of them will have an allowed range from 0 to 100, except
     * for hue, which has a limit of 360. */
    else if (parse_command_int(buffer, "hue", "?hue", 360, &(light.hue)))
        noop();
    else if (parse_command_int(buffer, "sat", "?sat", 100, &(light.saturation)))
        noop();
    else if (parse_command_int(buffer, "lum", "?lum", 100, &(light.luminance)))
        noop();
}


int
main()
{
    /* Initialize all components of the firmware and setup all required ports
     * and registers for service. */
    uart_init(); /* enable UART */
    sei();       /* enable interupts */

    /* Use an infinite loop for repeatedly update the PWM registers (i.e. to
     * fade colors) and check for new commands to be parsed executed. */
    while (true) {
        /* Finally, check the UART buffers for any new commands. If a new
         * command is available, it will be parsed and necessary steps executed
         * by the following function. */
        parse_command();
    }

    return 0;
}
