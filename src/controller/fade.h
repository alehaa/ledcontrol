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

#ifndef LEDCONTROL_FADE_H
#define LEDCONTROL_FADE_H


#include "config.h"


/**
 * Number of steps for fading.
 *
 * This number defines how many steps fading the light takes over the time
 * defined in @ref FADE_WAIT, i.e. how many variations of colors may lay between
 * the start and end colors.
 *
 * As each RGB channel uses 8 bit and therefore can have a maximum of 255
 * variations, this will be the fixed number of steps for fading.
 */
#define FADE_STEPS 255.0

/**
 * Time between two steps during fading.
 *
 * This macro defines the miliseconds between two steps during fading and is
 * calculated by dividing the time a fade process should take through the number
 * of steps.
 *
 * @note As other commands may be executed between two steps while fading, the
 *       actual time can differ slightly from the one defined in this macro.
 *       However, as this should only be a few microseconds, the human eye
 *       shouldn't recognize this.
 *
 * @warning During fading, the LED controller will become unresponsive (i.e.
 *          doesn't respond to any command via UART), as its waiting the fixed
 *          time defined in this macro between steps, before commands will be
 *          evaluated and the next step begins.
 */
#define FADE_WAIT ((LED_FADE_TIME * 1000) / FADE_STEPS)


#endif
