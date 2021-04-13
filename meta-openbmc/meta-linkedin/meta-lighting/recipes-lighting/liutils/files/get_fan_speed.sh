#!/bin/sh
#
# Copyright 2018-present LinkedIn`. All Rights Reserved.
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
    echo "Usage: $0 [Fan Unit (1..4)]" >&2
}

FAN1=/sys/class/i2c-adapter/i2c-8/8-0018/hwmon/hwmon*/fan1_input
FAN2=/sys/class/i2c-adapter/i2c-8/8-0019/hwmon/hwmon*/fan1_input
FAN3=/sys/class/i2c-adapter/i2c-8/8-001a/hwmon/hwmon*/fan1_input
FAN4=/sys/class/i2c-adapter/i2c-8/8-002c/hwmon/hwmon*/fan1_input

set -e

get_i2c_control 8 124

id=1
if [ "$#" -eq 0 ]; then
    FANS="$FAN1 $FAN2 $FAN3 $FAN4"
elif [ "$#" -eq 1 ]; then
    case "$1" in
    "1")
        FANS="$FAN1"
        id=1
        ;;
    "2")
        FANS="$FAN2"
        id=2
        ;;
    "3")
        FANS="$FAN3"
        id=3
    ;;
    "4")
        FANS="$FAN4"
        id=4
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

for fan in $FANS; do
    echo "Fan $id RPMs:    $(cat $fan)"
    let "id++"
done