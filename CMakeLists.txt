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

cmake_minimum_required(VERSION 2.8)


# Configure the toolchain to compile for an arduino nano as target platform.
# This is the intended platform for this project, but other targets can be
# configured as well.
#
# NOTE: All relevant options for the AVR toolchain need to be configured before
#       the CMake project is initialized (by calling the project function), as
#       this will test and initialize the toolchain with these parameters.
set(AVR_MCU   "atmega328p")
set(MCU_SPEED "16000000UL")
set(AVR_PROGRAMMER         "arduino")
set(AVR_UPLOADTOOL_PORT    "/dev/ttyUSB0" CACHE PATH "arduino device file")
set(AVR_UPLOADTOOL_OPTIONS "-b 57600")


# Initialize the project.
#
# NOTE: The entire toolchain needs to be configured before this function is
#       called.
project("ledcontrol" C)


# ===========================
# Recurse into subdirectories
# ===========================

add_subdirectory("src")
