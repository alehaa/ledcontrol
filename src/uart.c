/* This file is part of ledcontrol.
 *
 * ledcontrol is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Copyright (C)
 *  2016 Alexander Haase <ahaase@alexhaase.de>
 */

/* The code in this file was inspired by the Atmega328p manual and
 * http://www.appelsiini.net/2011/simple-usart-with-avr-libc
 */

#include "uart.h"

#include <stdio.h>

#include <avr/io.h>


#define BAUD 57600
#define UBRR F_CPU / 16 / BAUD - 1


/** \brief Init USART registers.
 *
 * \details This function sets all necessary register bits for the USART
 *  connection for 8n1 transmission with 9600 BAUD.
 */
void
uart_init()
{
	// set USART baud rate
	UBRR0H = (unsigned char)((UBRR) >> 8);
	UBRR0L = (unsigned char)(UBRR);

	// enable receiver and transmitter
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);

	// set frame format: 8n1
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}


/** \brief Setup file descriptors for stdin and stdout via UART.
 *
 * \details To use UART via printf and similar functions, we need to make UART
 *  available via global file descriptors. Later \ref uart_init_stdio will link
 *  them to stdin, stdout and stderr.
 */
FILE uart_in = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
FILE uart_out = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


/** \brief Setup stdin, stdout and stderr for usage with UART.
 *
 * \details This function will setup the global file descriptors \ref uart_in
 *  and \ref uart_out as stdin, stdout and stderr, so they can be used via
 *  normal printf and similar functions.
 */
void
uart_init_stdio()
{
	stdin = &uart_in;
	stdout = &uart_out;
	stderr = &uart_out;
}


/** \brief Send \p c over UART.
 *
 *
 * \param c Char to be send.
 * \param stream Pointer to sending stream.
 *
 * \return This function returns 0 after sucessful reading one byte.
 */
int
uart_putchar(char c, FILE *stream)
{
	while (!(UCSR0A & _BV(UDRE0)))
		;
	UDR0 = c;

	return 0;
}


/** \brief Receive one byte over UART.
 *
 *
 * \param stream Pointer to receiving stream.
 *
 * \return This function returns the received byte.
 */
int
uart_getchar(FILE *stream)
{
	while (!(UCSR0A & _BV(RXC0)))
		;

	return (int)UDR0;
}
