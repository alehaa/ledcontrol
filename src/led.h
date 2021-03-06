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

#ifndef LEDCONTROL_LED_H
#define LEDCONTROL_LED_H


#include <stddef.h>
#include <stdint.h>


typedef struct rgb
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb;


void ledcontrol_led_init();
void ledcontrol_led_write(rgb *color, int n);


#endif
