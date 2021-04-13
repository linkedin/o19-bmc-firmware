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
    echo "Usage: $0 [temperature Unit (1..4)]" >&2
}

FANCTRL_DIR=/sys/class/i2c-adapter/i2c-8/8-0030/
set -e

#i2cset -y 8 0x7c 0x1 0x07
get_i2c_control 8 124

if [ "$#" -eq 0 ]; then
    TEMPS="temperature1 temperature2 temperature3 temperature4"
elif [ "$#" -eq 1 ]; then
    case "$1" in
    "1")
        TEMPS="temperature1"
        ;;
    "2")
        TEMPS="temperature2"
        ;;
    "3")
        TEMPS="temperature3"
        ;;
    "4")
        TEMPS="temperature4"
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
    temp_var=$FANCTRL_DIR/$temp
    temp_val=$(cat $temp_var)
    unit=10
    val=$(( $temp_val/$unit ))
    echo "temperature $temp in C:    $val"
done

