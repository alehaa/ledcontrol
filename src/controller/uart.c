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

/* The code in this file was inspired by the Atmega328p manual and
 * https://www.mikrocontroller.net/articles/Interrupt */

#include "uart.h"
#include "config.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/io.h>


/* Generate UART settings by using the setbaud utilities. This will calculate
 * the related register values and modes required for running the arduino with
 * this specific baud rate.
 *
 * NOTE: The BAUD and F_CPU macros need to be set *before* the utility header is
 *       included, as it affects the calculation. */
#define BAUD LED_BAUD_RATE
#include <util/setbaud.h>


/**
 * UART receive ready flag.
 *
 * This flag is set, if a string has been received completly by the Interrupt
 * Service Routine and can be copied into the main program by calling @ref
 * uart_receive.
 */
static volatile bool uart_rx_ready = 0;

/**
 * UART transmission ready flag.
 *
 * This flag is set, if a new string can be copied in the transmission buffer by
 * calling @ref uart_send. It will be unset while there's an ongoing
 * transmission.
 */
static volatile bool uart_tx_ready = 1;


/**
 * Receive buffer.
 *
 * Any data received via UART is writtin into this buffer by an Interrupt
 * Service Routine, from where it can be copied into the main program by calling
 * @ref uart_receive.
 */
static char uart_rx_buffer[UART_BUFFER_SIZE];

/**
 * Transmission buffer.
 *
 * Any data to be transmitted will be copied into this buffer by @ref uart_send
 * before an Interrupt Service Routine will transmit the data via UART.
 */
static char uart_tx_buffer[UART_BUFFER_SIZE];


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

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);             /* 8-bit data */
    UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0); /* enable RX and TX */
}


/**
 * Data receive Interrupt Service Routine.
 *
 * This function provides the Interrupt Service Routine (ISR) to receive data
 * from UART and writing it into @ref uart_tx_buffer. It is triggered, if the
 * data register `UDR0` has received the next byte and is ready to be read.
 */
ISR(USART_RX_vect)
{
    /* Maintain a static counter, on which position to store a received byte in
     * the buffer. As only a single byte can be received via UART on each call,
     * this will iterate over all bytes of the buffer across multiple calls of
     * this ISR function. */
    static uint8_t uart_rx_cnt = 0;

    /* Read the received byte from the data register. This will remove the
     * interrupt flag, indicating this byte has been read and the next one could
     * be received or send by the atmega's firmware. */
    char data = UDR0;

    /* If the buffer can't handle new data, as it currently stores the last
     * received string, just exist the ISR function and discard the received
     * byte.
     *
     * NOTE: This needs to be done AFTER the data register has been read to
     *       clear the interrupt flag. Otherwise this ISR function will be
     *       called over and over again for the same byte without any chance for
     *       the main program to clear the receive buffer. */
    if (uart_rx_ready)
        return;

    /* If the end of the string is reached, terminate the string by a null
     * character and set the related ready flag, so the main program can copy
     * the received string. In addition, the internal counter gets reset, so the
     * next received data is stored at the begin of the buffer. */
    if (data == '\r' || data == '\n') {
        uart_rx_buffer[uart_rx_cnt] = '\0';
        uart_rx_ready = true;
        uart_rx_cnt = 0;
        return;
    }

    /* If the buffer is not filled completly yet, store the next byte in the
     * buffer and increment the counter for receiving the next byte. */
    if (uart_rx_cnt < (UART_BUFFER_SIZE - 1))
        uart_rx_buffer[uart_rx_cnt++] = data;
}


/**
 * Receive a string from UART.
 *
 * This function copys a string from @ref uart_rx_buffer into @p dst, which was
 * previously received by the data receive Interrupt Service Routine. This
 * allows the calling function to perform other tasks while receiving data and
 * copy the data efficiently, when a whole line has been received.
 *
 *
 * @param dst Where to copy the data.
 * @param len The length of @p dst.
 *
 * @return On success this function returns true. If the read buffer is not
 *         ready yet (i.e. the data has not been received completly yet), false
 *         will be returned.
 */
bool
uart_receive(char *dst, size_t len)
{
    /* If no string has been received completly yet, return false as error code
     * indicating no data can be read from the receive buffer right now.
     *
     * NOTE: This check is not thread-safe, as two threads could evaluate this
     *       variable simultanous and continue, before one of them sets the
     *       flag. However, as the main program is single-threaded only, there
     *       shouldn't be any race conditions. */
    if (!uart_rx_ready)
        return false;

    /* Copy the buffer into the provided destination and reset the ready flag,
     * so the next string can be received by the ISR function. */
    strncpy(dst, uart_rx_buffer, len);
    uart_rx_ready = false;
    return 1;
}


/**
 * Data transmission Interrupt Service Routine.
 *
 * This function provides the Interrupt Service Routine (ISR) to transmit data
 * of @ref uart_tx_buffer via UART. It is triggered, if the send register `UDR0`
 * is ready to send the next byte.
 */
ISR(USART_UDRE_vect)
{
    /* Maintain a static pointer to the current position in the send buffer,
     * which is incremented on each call of this ISR function. As only a single
     * byte can be sent via UART on each call, this will iterate over all bytes
     * in the buffer across multiple calls of this ISR function.
     *
     * NOTE: The increment does NOT check for buffer overflows. Therefore, any
     *       data to be sent needs a terminating null character to stop the
     *       transmission. */
    static char *uart_tx_p = uart_tx_buffer;
    char data = *uart_tx_p++;

    /* If the end of the buffer is reached, disable the UDRE interrupt, so this
     * function is not called until there's new data available for transmission.
     * In addition, the internal pointer gets reset, so the next transmission
     * starts from the begin of the buffer. */
    if (data == '\0') {
        UCSR0B &= ~(_BV(UDRIE0));
        uart_tx_p = uart_tx_buffer;
        uart_tx_ready = true;
        return;
    }

    /* Send the current byte via UART by simply writing the value into the UART
     * input / output register. The atmega's firmware will then take and
     * transmit the byte automatically. */
    UDR0 = data;
}


/**
 * Send a string via UART.
 *
 * This function copys the given string @p src into the send buffer @ref
 * uart_tx_buffer, before starting an asynchronous UART transmission. It will
 * return after the data has been copied into the buffer, so other tasks can be
 * performed while the data is transmitted.
 *
 *
 * @param src The data to be copied. Needs to have a terminating null character.
 *
 * @return On success this function returns true. If the send buffer is not
 *         ready yet (i.e. another transmission has not been finished yet),
 *         false will be returned.
 */
bool
uart_send(const char *src)
{
    /* If the UART connection is not ready for transmitting new data, i.e. as
     * data currently is being sent, return false as error code indicating no
     * data can be written to the send buffer right now.
     *
     * NOTE: This check is not thread-safe, as two threads could evaluate this
     *       variable simultanous and continue, before one of them sets the
     *       flag. However, as the main program is single-threaded only, there
     *       shouldn't be any race conditions. */
    if (!uart_tx_ready)
        return false;

    /* Lock the UART send buffer before copying the passed string into the UART
     * send buffer. Finally, the send interupt will be enabled to start sending
     * the buffer contents via UART and return a success status code. */
    uart_tx_ready = false;
    strncpy(uart_tx_buffer, src, UART_BUFFER_SIZE);
    UCSR0B |= _BV(UDRIE0);
    return true;
}
