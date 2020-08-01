# This file is part of ledcontrol.
#
# ledcontrol is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful,but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
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
set(AVR_BAUD_RATE 9600 CACHE STRING "Baud rate for controller UART")

# Generate a configuration file, which can be included and used by all source
# code files of the controller's firmware.
configure_file("config.h.in" "config.h")
include_directories(${CMAKE_CURRENT_BINARY_DIR})


# Compiler options for all builds
#
add_definitions("-DF_CPU=${MCU_SPEED}")


add_avr_executable(ledcontrol
	uart.c
	main.c)
