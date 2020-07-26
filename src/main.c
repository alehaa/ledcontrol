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

#include <stdbool.h>
#include <stdio.h>

#include <util/delay.h>

#include "led.h"
#include "uart.h"


int
main(int argc, char **argv)
{
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

	return 0;
}
