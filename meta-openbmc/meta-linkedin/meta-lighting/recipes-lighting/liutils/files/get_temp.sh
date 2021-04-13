#!/bin/sh
#
# Copyright 2018-present LinkedIn. All Rights Reserved.
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
PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin
. /usr/local/bin/set_control.sh

usage() {
    echo "Usage: $0 [temperature Unit (1..12)]" >&2
}

set -e

TEMP1="/sys/class/i2c-adapter/i2c-6/6-0070/hwmon/hwmon*/temp1_input"
TEMP2="/sys/class/i2c-adapter/i2c-6/6-0071/hwmon/hwmon*/temp1_input"
TEMP3="/sys/class/i2c-adapter/i2c-6/6-0072/hwmon/hwmon*/temp1_input"
TEMP4="/sys/class/i2c-adapter/i2c-6/6-0073/hwmon/hwmon*/temp1_input"
TEMP5="/sys/class/i2c-adapter/i2c-6/6-0074/hwmon/hwmon*/temp1_input"
TEMP6="/sys/class/i2c-adapter/i2c-6/6-0075/hwmon/hwmon*/temp1_input"
TEMP7="/sys/class/i2c-adapter/i2c-6/6-0076/hwmon/hwmon*/temp1_input"
TEMP8="/sys/class/i2c-adapter/i2c-6/6-0077/hwmon/hwmon*/temp1_input"
TEMP9="/sys/class/i2c-adapter/i2c-6/6-0028/hwmon/hwmon*/temp1_input"
TEMP10="/sys/class/i2c-adapter/i2c-6/6-0029/hwmon/hwmon*/temp1_input"
TEMP11="/sys/class/i2c-adapter/i2c-6/6-002a/hwmon/hwmon*/temp1_input"
TEMP12="/sys/class/i2c-adapter/i2c-6/6-002b/hwmon/hwmon*/temp1_input"

get_i2c_control 6 126

id=1
if [ "$#" -eq 0 ]; then
    TEMPS="$TEMP1 $TEMP2 $TEMP3 $TEMP4 $TEMP5 $TEMP6 $TEMP7 $TEMP8 $TEMP9 $TEMP10 $TEMP11 $TEMP12"
elif [ "$#" -eq 1 ]; then
    case "$1" in
    "1")
        TEMPS="$TEMP1"
        id=1
        ;;
    "2")
        TEMPS="$TEMP2"
        id=2
        ;;
    "3")
        TEMPS="$TEMP3"
        id=3
        ;;
    "4")
        TEMPS="$TEMP4"
        id=4
        ;;
    "5")
        TEMPS="$TEMP5"
        id=5
        ;;
    "6")
        TEMPS="$TEMP6"
        id=6
        ;;
    "7")
        TEMPS="$TEMP7"
        id=7
        ;;
    "8")
        TEMPS="$TEMP8"
        id=8
        ;;
    "9")
        TEMPS="$TEMP9"
        id=9
        ;;
    "10")
        TEMPS="$TEMP10"
        id=10
        ;;
    "11")
        TEMPS="$TEMP11"
        id=11
        ;;
    "12")
        TEMPS="$TEMP12"
        id=12
        ;;
    *)
        usage
        exit 1
        ;;
    esac
else
    usage
    exit 1
fi

for temp in $TEMPS; do
    temp_var=$temp
    temp_val=$(cat $temp_var)
    unit=1000
    val=$(( $temp_val/$unit ))
    echo "sensor $id temp in C:    $val"
    let "id++"
done

