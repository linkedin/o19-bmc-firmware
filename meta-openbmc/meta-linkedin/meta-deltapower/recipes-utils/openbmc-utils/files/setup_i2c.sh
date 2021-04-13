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

# Bus 0 eFuse
#i2cset -y 0 0x79 0x1 0x07
for i in {1..24};
    do
        usleep 25000
        get_i2c_control 0 121
        let "idx = $i - 1"
        addr=0x${eFuse_addrs[$idx]}
        i2c_device_add 0 ${addr} lm25066

        #check if driver is installed
        dir=$(get_eFuse_dir $i)
        efuse_file=$dir/control/shutdown
        counter=20
        while [ $counter -gt 0 ];
        do
           if [ ! -f $efuse_file ]; then
               get_i2c_control 0 121
               check_eFuse_driver $i
               usleep 25000
           else
              break
           fi

           counter=$(( $counter - 1 ))
        done
    done

# Bus 1 PSU2
#i2cset -y 1 0x72 0x1 0x07
get_i2c_control 1 114
i2c_device_add 1 0x40 psu            # power supply
i2c_device_add 1 0x57 24c64          # power supply EEPROM

# Bus 2 PSU3
#i2cset -y 2 0x73 0x1 0x07
get_i2c_control 2 115
i2c_device_add 2 0x40 psu            # power supply
i2c_device_add 2 0x57 24c64          # power supply EEPROM

# Bus 3 PSU4
#i2cset -y 3 0x74 0x1 0x07
get_i2c_control 3 116
i2c_device_add 3 0x40 psu            # power supply
i2c_device_add 3 0x57 24c64          # power supply EEPROM

# Bus 4
#i2cset -y 4 0x7a 0x1 0x07
#get_i2c_control 4 122

for i in {25..50};
    do
        usleep 25000
        get_i2c_control 4 122
        let "idx = $i - 1"
        addr=0x${eFuse_addrs[$idx]}
        i2c_device_add 4 ${addr} lm25066

        #check if driver is installed
        dir=$(get_eFuse_dir $i)
        efuse_file=$dir/control/shutdown
        counter=20
        while [ $counter -gt 0 ]
        do
           if [ ! -f $efuse_file ]; then
               get_i2c_control 4 122
               check_eFuse_driver $i
               usleep 50000
           else
               break
           fi
           counter=$(( $counter - 1 ))
        done
    done

# Bus 6 PSU5
#i2cset -y 6 0x75 0x1 0x07
get_i2c_control 6 117
i2c_device_add 6 0x40 psu            # power supply
i2c_device_add 6 0x57 24c64          # power supply EEPROM

# Bus 7 PSU6
#i2cset -y 7 0x76 0x1 0x07
get_i2c_control 7 118
i2c_device_add 7 0x40 psu            # power supply
i2c_device_add 7 0x57 24c64          # power supply EEPROM

# Bus 8
#i2cset -y 8 0x7c 0x1 0x07
get_i2c_control 8 124
i2c_device_add 8 0x30 fancontrol
i2c_device_add 8 0x50 24c02          # BMC PHY EEPROM

# Bus 12 PSU1
#i2cset -y 12 0x71 0x1 0x07
get_i2c_control 12 113
i2c_device_add 12 0x40 psu            # power supply
i2c_device_add 12 0x57 24c64          # power supply EEPROM
