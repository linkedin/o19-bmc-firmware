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
    echo "Usage: $0 [Fan Unit (1..4)]" >&2
}

FANCTRL_DIR=/sys/class/i2c-adapter/i2c-8/8-0030/
set -e

get_i2c_control 8 124

if [ "$#" -eq 0 ]; then
    FANS="fan1_speed fan2_speed fan3_speed fan4_speed"
elif [ "$#" -eq 1 ]; then
    case "$1" in
    "1")
        FANS="fan1_speed"
        ;;
    "2")
        FANS="fan2_speed"
        ;;
    "3")
        FANS="fan3_speed"
        ;;
    "4")
        FANS="fan4_speed"
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
    echo "Fan $fan RPMs:    $(cat $FANCTRL_DIR/$fan)"
done
