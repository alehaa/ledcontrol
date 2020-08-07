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

#include "pwm.h"

#include <avr/io.h>


/**
 * Initialize PWM registers.
 *
 * This function enables the PWM functionality of the atmega CPU and prepares
 * the necessary outputs to function as PWM outputs.
 */
void
pwm_init()
{
    /* Enable the PWM mode as alternate function for port D pins D3, D5 and D6
     * and assign them to the related PWM counters OC2B, OC0A and OC0B. */
    DDRD = _BV(PD3) | _BV(PD5) | _BV(PD6);

    /* There are quite a number of PWM modes available but for the sake of
     * simplicity we'll just use the 8-bit Fast PWM mode. This is done by
     * setting the WGM00 and WGM01 (or WGM21 for OC2) bits. Setting the COM
     * flags tells the microcontroller to set the PWM outputs low when the
     * timer's counter reaches a compare value (i.e. the values defined in the
     * PWM registers PWM_R, PWM_G and PWM_B). */
    TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00) | _BV(WGM01); /* OC0 */
    TCCR2A = _BV(COM2B1) | _BV(WGM20) | _BV(WGM21);               /* OC2 */

    /* Set the CS flags to turn on the timer without a prescaler (so at full
     * speed). The timer is used to determine when the PWM pin should be on and
     * when it should be off. */
    TCCR0B = _BV(CS00); /* OC0 */
    TCCR2B = _BV(CS20); /* OC2 */
}


/**
 * Get the current PWM output values.
 *
 * This function gets the PWM output values currently set. Its intended use is
 * to receive the current state, so a transition from the current color to the
 * next one can be calculated and the light can fade to it.
 *
 *
 * @return The current state of the light as @ref rgb color.
 */
rgb
pwm_get_rgb()
{
    rgb tmp = {.r = PWM_R, .g = PWM_G, .b = PWM_B};
    return tmp;
}
