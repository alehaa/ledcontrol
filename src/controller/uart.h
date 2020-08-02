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

#ifndef LEDCONTROL_UART_H
#define LEDCONTROL_UART_H


#include <stdbool.h>
#include <stddef.h>


/**
 * The UART read and send buffer size.
 *
 * This size can be used to define individual buffers depending on the buffer
 * sizes of the UART functions.
 *
 * NOTE: Beware, that the maximum buffer size is limited to the used `uint8_t`
 *       type in UART functions. Therefore, this size must not exceed 256 bytes.
 */
#define UART_BUFFER_SIZE 32


void uart_init();
bool uart_receive(char *dst, size_t len);
bool uart_send(const char *src);


#endif
