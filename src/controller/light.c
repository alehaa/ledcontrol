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

#include "light.h"

#include <math.h>


/**
 * Current desired configuration for the light.
 *
 * This struct stores the target configuration of the light, which it will fade
 * to and keep thereafter.
 */
light_status light = {0};


/**
 * Generate a RGB color struct from individual channels.
 *
 * This function can be used to create a new RGB color struct from individual
 * channels. Its intended use is to generate these structs for directly
 * returning them in other functions, instead of having to assign the struct
 * manually each time.
 *
 * @note All parameters accept percentages between 0 and 1 only, as this is the
 *       calculation range of the algorithm used in @ref hsv2rgb, which
 *       primarily uses this function.
 *
 *
 * @param r Color channel percentage for red.
 * @param g Color channel percentage for green.
 * @param b Color channel percentage for blue.
 *
 * @return The generated color struct.
 */
static rgb
make_rgb(float r, float g, float b)
{
    rgb tmp = {r * 255, g * 255, b * 255};
    return tmp;
}


/**
 * Convert a color from HSV to RGB.
 *
 * This function takes the three parameters of a color defined as HSV and
 * generates the related RGB color.
 *
 * @todo Currently this function uses floats, which require a lot of helper
 *       functions for calculation. This function should be optimize to use
 *       integers only, if possible.
 *
 *
 * @param h HSV channel for hue.
 * @param s HSV channel for saturation.
 * @param v HSV channel for value (brightness).
 *
 * @return The HSV color converted to RGB.
 */
static rgb
hsv2rgb(float h, float s, float v)
{
    /* This implementation is based on the "Educational Color Applets Homepage"
     * by Eugene Vishnevsky. https://www.cs.rit.edu/~ncs/color/t_convert.html */

    /* If the given color has no saturation at all, its just a shade of grey.
     * Therefore, the related RGB color just uses the value (brightness) for all
     * of the three indivudal channels. */
    if (s == 0)
        return make_rgb(v, v, v);

    /* In any other case, the individual colors will be calculated by the HSV to
     * RGB convertion algorithm below. It checks the sector of hue and assign
     * the RGB channels depending on this sector by calculated values. */
    h /= 60;
    int i = floor(h);
    float f = h - i;
    float p = v * (1 - s);
    float q = v * (1 - s * f);
    float t = v * (1 - s * (1 - f));
    switch (i) {
        case 0: return make_rgb(v, t, p);
        case 1: return make_rgb(q, v, p);
        case 2: return make_rgb(p, v, t);
        case 3: return make_rgb(p, q, v);
        case 4: return make_rgb(t, p, v);
        default: return make_rgb(v, p, q);
    }
}


/**
 * Get the RGB color codes for the currently configured color of the light.
 *
 * The internal configuration of the light just stores the HSV data, as these
 * values are required for handling API calls. Whenever one of the configured
 * values is altered, this function can be used to convert the HSV configuration
 * to RGB colors. It is required, as the light controller uses three individual
 * PWM channels as outputs to connect an RGB LED strip.
 *
 *
 * @return The current light configuration from @ref light converted to RGB.
 */
rgb
light_rgb()
{
    /* If the light is not powered on at all, obviously the output color is
     * black. Therefore no color needs to be converted and this function can
     * return immediately. */
    if (!light.power)
        return make_rgb(0, 0, 0);

    /* Convert the current light configuration from HSV to RGB and return it. As
     * the hsl2rgb function awaits saturation and value to be percentages, these
     * will be converted before passing them as arguments. */
    return hsv2rgb(light.hue, light.saturation / 100.0, light.value / 100.0);
}
