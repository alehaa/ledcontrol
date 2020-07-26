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

/* The following code is a bootstraped version of light_ws2812.
 *  Author: Tim (cpldcpu@gmail.com)
 *  License: GNU GPL v2 (see License.txt)
 */

#include "led.h"

#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>


#define WS2812_PORTREG PORTB
#define WS2812_DDRREG DDRB
#define WS2812_PORT 2

// Timing in ns
#define w_zeropulse 350
#define w_onepulse 900
#define w_totalperiod 1250

// Fixed cycles used by the inner loop
#define w_fixedlow 2
#define w_fixedhigh 4
#define w_fixedtotal 8

// Insert NOPs to match the timing, if possible
#define w_zerocycles (((F_CPU / 1000) * w_zeropulse) / 1000000)
#define w_onecycles (((F_CPU / 1000) * w_onepulse + 500000) / 1000000)
#define w_totalcycles (((F_CPU / 1000) * w_totalperiod + 500000) / 1000000)

// w1 - nops between rising edge and falling edge - low
#define w1 (w_zerocycles - w_fixedlow)
// w2   nops between fe low and fe high
#define w2 (w_onecycles - w_fixedhigh - w1)
// w3   nops to complete loop
#define w3 (w_totalcycles - w_fixedtotal - w1 - w2)

#if w1 > 0
#define w1_nops w1
#else
#define w1_nops 0
#endif

// The only critical timing parameter is the minimum pulse length of the "0"
// Warn or throw error if this timing can not be met with current F_CPU
// settings.
#define w_lowtime ((w1_nops + w_fixedlow) * 1000000) / (F_CPU / 1000)
#if w_lowtime > 550
#error \
    "Light_ws2812: Sorry, the clock speed is too low. Did you set F_CPU correctly?"
#elif w_lowtime > 450
#warning \
    "Light_ws2812: The timing is critical and may only work on WS2812B, not on WS2812(S)."
#warning "Please consider a higher clockspeed, if possible"
#endif

#if w2 > 0
#define w2_nops w2
#else
#define w2_nops 0
#endif

#if w3 > 0
#define w3_nops w3
#else
#define w3_nops 0
#endif

#define w_nop1 "nop      \n\t"
#define w_nop2 "rjmp .+0 \n\t"
#define w_nop4 w_nop2 w_nop2
#define w_nop8 w_nop4 w_nop4
#define w_nop16 w_nop8 w_nop8


void
ledcontrol_led_init()
{
	// Enable output
	WS2812_DDRREG |= _BV(WS2812_PORT);
}


static inline void
ledcontrol_led_sendbyte(const uint8_t data, const uint8_t masklo,
                        const uint8_t maskhi)
{
	uint8_t ctr;

	asm volatile("       ldi   %0,8  \n\t"
	             "loop%=:            \n\t"
	             "       out   %2,%3 \n\t"
#if (w1_nops & 1)
	             w_nop1
#endif
#if (w1_nops & 2)
	                 w_nop2
#endif
#if (w1_nops & 4)
	                     w_nop4
#endif
#if (w1_nops & 8)
	                         w_nop8
#endif
#if (w1_nops & 16)
	                             w_nop16
#endif
	             "       sbrs  %1,7  \n\t"
	             "       out   %2,%4 \n\t"
	             "       lsl   %1    \n\t"
#if (w2_nops & 1)
	             w_nop1
#endif
#if (w2_nops & 2)
	                 w_nop2
#endif
#if (w2_nops & 4)
	                     w_nop4
#endif
#if (w2_nops & 8)
	                         w_nop8
#endif
#if (w2_nops & 16)
	                             w_nop16
#endif
	             "       out   %2,%4 \n\t"
#if (w3_nops & 1)
	             w_nop1
#endif
#if (w3_nops & 2)
	                 w_nop2
#endif
#if (w3_nops & 4)
	                     w_nop4
#endif
#if (w3_nops & 8)
	                         w_nop8
#endif
#if (w3_nops & 16)
	                             w_nop16
#endif

	             "       dec   %0    \n\t"
	             "       brne  loop%=\n\t"
	             : "=&d"(ctr)
	             : "r"(data), "I"(_SFR_IO_ADDR(WS2812_PORTREG)), "r"(maskhi),
	               "r"(masklo));
}


void
ledcontrol_led_write(rgb *color, int n)
{
	// Save status register and disable interrupts.
	uint8_t sreg_save = SREG;
	cli();

	uint8_t masklo = ~(_BV(WS2812_PORT)) & WS2812_PORTREG;
	uint8_t maskhi = _BV(WS2812_PORT) | WS2812_PORTREG;

	do {
		ledcontrol_led_sendbyte(color->g, masklo, maskhi);
		ledcontrol_led_sendbyte(color->r, masklo, maskhi);
		ledcontrol_led_sendbyte(color->b, masklo, maskhi);
	} while (--n);


	// Reset status register.
	SREG = sreg_save;
}
