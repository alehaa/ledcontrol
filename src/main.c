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

#include <avr/io.h>
#include <util/delay.h>


#define fade_up(color, color_register) \
	while (++color < 256) {            \
		color_register = color;        \
		_delay_ms(20);                 \
	}

#define fade_down(color, color_register) \
	while (--color > 0x17) {             \
		color_register = color;          \
		_delay_ms(20);                   \
	}

#define RED OCR0A
#define GREEN OCR0B
#define BLUE OCR2B


int
main(int argc, char **argv)
{
	//	DDRB |= _BV(PB1); /* OC1B */
	DDRD |= _BV(PD3) | _BV(PD5) | _BV(PD6); /* OC2B, OC0A & OC0B */

	/* Set the timer for OC0 & OC2 */
	TCCR0A |= _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00) | _BV(WGM01);
	TCCR0B |= _BV(CS00);

	TCCR2A |= _BV(COM2B1) | _BV(WGM20) | _BV(WGM21);
	TCCR2B |= _BV(CS20);


/* There are quite a number of PWM modes available but for the sake of
 * simplicity we'll just use the 8-bit Fast PWM mode. This is done by
 * setting the WGM10 and WGM12 bits. Setting COM1A1 tells the micro-
 * controller to set the output of the OCR1A pin low when the timer's
 * counter reaches a compare value (which will be explained below). CS10
 * being set simply turns the timer on without a prescaler (so at full
 * speed). The timer is used to determine when the PWM pin should be on and
 * when it should be off. */
#if 0
	TCCR1A |= _BV(COM1A1) | _BV(WGM10);
	TCCR1B |= _BV(CS10) | _BV(WGM12);
#endif

#if 1
	/* This loop is used to change the value in the OCR1A register. What that
	 * means is we're telling the timer waveform generator the point when it
	 * should change the state of the PWM pin. The way we configured it (with
	 * _BV(COM1A1) above) tells the generator to have the pin be on when the
	 * timer is at zero and then to turn it off once it reaches the value in the
	 * OCR1A register.
	 *
	 * Given that we are using an 8-bit mode the timer will reset to zero after
	 * it reaches 0xff, so we have 255 ticks of the timer until it resets. The
	 * value stored in OCR1A is the point within those 255 ticks of the timer
	 * when the output pin should be turned off (remember, it starts on).
	 *
	 * Effectively this means that the ratio of pwm / 255 is the percentage of
	 * time that the pin will be high.  Given this it isn't too hard to see what
	 * when the pwm value is at 0x00 the LED will be off and when it is 0xff the
	 * LED will be at its brightest. */
	int r = 0, g = 0, b = 0;
	while (1) {
		fade_up(r, RED);
		fade_down(b, BLUE);
		fade_up(g, GREEN);
		fade_down(r, RED);
		fade_up(b, BLUE);
		fade_down(g, GREEN);
	}
#endif

	while (1) {
		RED = 0xff;
		GREEN = 0xff;
		BLUE = 0xff;

		_delay_ms(1000);
	}

#if 0
	uart_init();
	uart_init_stdio();
	ledcontrol_led_init();

	rgb color;
	while (true) {
		if (!(fscanf(stdin, "%2x%2x%2x", &color.r, &color.g, &color.b) == 3)) {
			char c;
			while ((c = getchar()) && (c != '\n') && (c != '\r'))
				;

			continue;
		}

		size_t n;
		for (n = 0; n < 94; n++)
			ledcontrol_led_write(&color, 1);
		_delay_us(50);
	}
#endif

	return 0;
}
