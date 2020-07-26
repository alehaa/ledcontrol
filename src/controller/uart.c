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
 *  2016-2020 Alexander Haase <ahaase@alexhaase.de>
 */

/* The code in this file was inspired by the Atmega328p manual and
 * http://www.appelsiini.net/2011/simple-usart-with-avr-libc
 */

#include "uart.h"
#include "config.h"

#include <stdio.h>

#include <avr/io.h>


/* Generate UART settings by using the setbaud utilities. This will calculate
 * the related register values and modes required for running the arduino with
 * this specific baud rate.
 *
 * NOTE: The BAUD and F_CPU macros need to be set *before* the utility header is
 *       included, as it affects the calculation. */
#define BAUD AVR_BAUD_RATE
#include <util/setbaud.h>


/**
 * Init USART registers.
 *
 * This function sets all necessary register bits for the USART connection for
 * 8n1 transmission with the BAUD rate globally defined.
 */
void
uart_init()
{
    /* Set the baud rate registers with the values calculated by the related
     * utility header included above. */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    /* If the utility header determined, running the CPU in double speed is
     * required to provide the requested baud rate, double speed will be
     * configured by setting the related register, too. */
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif


    /**************************
     * additional configuration
     **************************/

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}


/**
 * Send character @p c via UART.
 *
 *
 * @param c      Char to be send.
 * @param stream Pointer to sending stream.
 *
 * @return This function returns 0 after sucessful reading one byte.
 */
static int
uart_putchar(char c, FILE *stream)
{
    /* Wait until data register empty, before pushing the next character byte
     * into the send buffer register. */
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;

    return 0;
}


/**
 * Receive one byte via UART.
 *
 *
 * @param stream Pointer to receiving stream.
 *
 * @return This function returns the received byte.
 */
static int
uart_getchar(FILE *stream)
{
    /* Wait until data exists in the receive buffer register, before passing it
     * to the calling function. */
    loop_until_bit_is_set(UCSR0A, RXC0);
    return (int)UDR0;
}


/**
 * Setup file descriptors for stdin and stdout via UART.
 *
 * To use UART via printf and similar functions, we need to make UART available
 * via global file descriptors. Later @ref uart_init_stdio will link them to
 * stdin, stdout and stderr.
 */
FILE uart_in = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
FILE uart_out = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);


/**
 * Setup stdin, stdout and stderr for usage with UART.
 *
 * This function will setup the global file descriptors @ref uart_in and @ref
 * uart_out as stdin, stdout and stderr, so they can be used via normal printf
 * and similar functions.
 */
void
uart_init_stdio()
{
    stdin = &uart_in;
    stdout = &uart_out;
    stderr = &uart_out;
}
