#!/bin/bash
#
# Copyright 2014-present Facebook. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin
. /usr/local/bin/ast-functions
. /usr/local/bin/set_control.sh

random_num=$RANDOM
delay_val=$((random_num*1000))
usleep $delay_val

# Bus 6 temp sensors 0x7e
get_i2c_control 6 126
i2c_device_add 6 0x70 lm75
i2c_device_add 6 0x71 lm75
i2c_device_add 6 0x72 lm75
i2c_device_add 6 0x73 lm75
i2c_device_add 6 0x74 lm75
i2c_device_add 6 0x75 lm75
i2c_device_add 6 0x76 lm75
i2c_device_add 6 0x77 lm75
i2c_device_add 6 0x28 lm75
i2c_device_add 6 0x29 lm75
i2c_device_add 6 0x2a lm75
i2c_device_add 6 0x2b lm75

# Bus 7 PSUs(1-3) 0x70
get_i2c_control 3 113
i2c_device_add 3 0x10 psu            # PSU1
i2c_device_add 3 0x50 psu            # PSU1

# Bus 8 0x7C
get_i2c_control 8 124
i2c_device_add 8 0x18 amc6821
i2c_device_add 8 0x19 amc6821
i2c_device_add 8 0x1a amc6821
i2c_device_add 8 0x2c amc6821

i2c_device_add 8 0x50 24c02          # BMC PHY EEPROM
