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

# Bus 1 eFuse 1 to 24
#i2cset -y 0 0x79 0x1 0x07
for i in {1..24};
    do
        get_i2c_control 1 121
        let "idx = $i - 1"
        addr=0x${eFuse_addrs[$idx]}
        i2c_device_delete 1 ${addr}
        i2c_device_add 1 ${addr} lm25066
    done

# Bus 2 efuse 25 - 50
for i in {25..50};
    do
        get_i2c_control 2 122
        let "idx = $i - 1"
        addr=0x${eFuse_addrs[$idx]}
        i2c_device_delete 2 ${addr}
        i2c_device_add 2 ${addr} lm25066
    done
