# This file is part of ledcontrol.
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.
#
#
# Copyright (C)
#  2016-2020 Alexander Haase <ahaase@alexhaase.de>

# Define the baud rate for UART communication with the arduino. The default
# setting of 9600 baud should fit most needs, as its the default for almost any
# chipset. Although its quite slow, it should fit the needs for carrying the few
# characters of the protocol implemented by the controller in a reasonable time.
set(LED_BAUD_RATE 9600 CACHE STRING "Baud rate for controller UART")

# Define the time fading the light should take. The default setting of 3 seconds
# should result in a smooth transition between very dark and light colors, but
# may be too long for slight changes in colors. It can be altered to any number
# of seconds. However, see the notes on FADE_WAIT before increasing.
set(LED_FADE_TIME 3 CACHE STRING "Time the light should fade to the new color")

# By default, the LED controller will control RGB LED stips. However, this
# behaviour can be disabled, to control the channels as individual lights, i.e.
# for connecting multiple monochrome LED strips to a single LED controller.
set(LED_RGB ON CACHE BOOL "Enable the RGB mode of the controller")

# Generate a configuration file, which can be included and used by all source
# code files of the controller's firmware.
configure_file("config.h.in" "config.h")
include_directories(${CMAKE_CURRENT_BINARY_DIR})


# Compiler options for all builds
#
add_definitions("-DF_CPU=${MCU_SPEED}")


add_avr_executable(ledcontrol main.c light.c pwm.c uart.c)
