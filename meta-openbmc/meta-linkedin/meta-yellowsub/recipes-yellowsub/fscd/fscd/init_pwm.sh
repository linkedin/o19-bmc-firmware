#!/bin/sh
#
# Copyright 2015-present Facebook. All Rights Reserved.
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
#
PWM_DIR=/sys/class/i2c-adapter/i2c-12/12-0031

set -e

# On Yellowsub, there are 4 fans connected.

# Fan PWM control register in system CPLD:
#     0x10 - 35%

# For each fan, setting to 35% initially
for pwm in 1 4; do
    echo 10 > $PWM_DIR/fan${pwm}_pwm_contrl
done
