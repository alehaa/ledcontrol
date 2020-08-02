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

#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"


int
main(int argc, char **argv)
{
    /* Initialize all components of the firmware and setup all required ports
     * and registers for service. */
    uart_init(); /* enable UART */
    sei();       /* enable interupts */


    char buffer[UART_BUFFER_SIZE];
    char b[UART_BUFFER_SIZE];
    while (true) {
        if (uart_receive(buffer, UART_BUFFER_SIZE)) {
            snprintf(b, 64, "got message: %s\n", buffer);
            uart_send(b);
        }
    }

    return 0;
}
