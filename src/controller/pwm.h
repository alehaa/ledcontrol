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

#ifndef LEDCONTROL_PWM_H
#define LEDCONTROL_PWM_H


#include <stdint.h>

#include <avr/io.h>

#include "light.h"


/**
 * PWM output register for color red.
 */
#define PWM_R OCR0B

/**
 * PWM output register for color green.
 */
#define PWM_G OCR2B

/**
 * PWM output register for color blue.
 */
#define PWM_B OCR0A


/**
 * Set PWM registerss for a specific RGB color.
 *
 * This macro sets the individual PWM registers for a specific RGB color. To
 * avoid passing an extra @ref rgb structure, the channels can be defined as
 * individual parameters.
 *
 * @note As this function implements a time critical operation, which is
 *       repeatedly called during fading, its implemented in the header file, to
 *       allow the compiler to inline this function.
 *
 *
 * @param r Color channel for red.
 * @param g Color channel for green.
 * @param b Color channel for blue.
 */
static inline void
pwm_set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    PWM_R = r;
    PWM_G = g;
    PWM_B = b;
}


void pwm_init();
rgb pwm_get_rgb();


#endif
