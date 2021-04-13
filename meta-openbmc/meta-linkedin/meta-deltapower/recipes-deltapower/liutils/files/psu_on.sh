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
PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin
. /usr/local/bin/set_control.sh

usage() {
    echo "Usage: $0 [psu unit [1..6]]" >&2
}

DIR1=/sys/class/i2c-adapter/

psu_addr[0]=$DIR1/i2c-12/12-0040
psu_addr[1]=$DIR1/i2c-1/1-0040
psu_addr[2]=$DIR1/i2c-2/2-0040
psu_addr[3]=$DIR1/i2c-3/3-0040
psu_addr[4]=$DIR1/i2c-6/6-0040
psu_addr[5]=$DIR1/i2c-7/7-0040

set -e

get_i2c_control 12 113
get_i2c_control 1 114
get_i2c_control 2 115
get_i2c_control 3 116
get_i2c_control 6 117
get_i2c_control 0 121

if [ "$#" -eq 0 ]; then
    usage
elif [ "$#" -eq 1 ]; then
    if [ $1 -lt 1 -o $1 -gt 6 ]; then
        echo "PSU unit out of range"
        usage
        exit 1
    fi

    case "$1" in
    "1"|"2"|"3"|"4"|"5"|"6")
        PSU=${psu_addr[$1 - 1]}
        ;;
    *)
        usage
        exit 1
        ;;
    esac

    echo "Turning on PSU $1 $PSU"
    echo 0x80 > $PSU/operation

else
    usage
    exit 1
fi
