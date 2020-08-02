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

#ifndef LEDCONTROL_LIGHT_H
#define LEDCONTROL_LIGHT_H


#include <stdbool.h>


/**
 * State configuration for the light.
 *
 * This struct type can be used to configure a given state of the light or save
 * previous ones. As most home automation systems use the HSL color system, this
 * struct reflects the required parameters for this system, too. Its values
 * should be sufficient to generate the RGB values if required.
 */
typedef struct
{
    bool power;     /**< The light's power status. */
    int hue;        /**< The light's hue. */
    int saturation; /**< The light's saturation. */
    int luminance;  /**< The light's luminance. */
} light_status;


extern light_status light;


#endif