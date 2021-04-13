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
. /usr/local/bin/ast-functions
. /usr/local/bin/set_control.sh

usage() {
    echo "Usage: $0 [eFuse# [1..50]]" >&2
}

get_i2c_control 0 121
get_i2c_control 4 122

if [ "$#" -eq 0 ]; then
    usage
    exit 1
elif [ "$#" -eq 1 ]; then
    if [ "$1" == "all" -o "$1" == "ALL" ]; then
        echo "info for all eFuses:"
        eFuse-util all
        exit 0
    fi

    if ! [[ "$1" =~ ^[0-9]+$ ]]; then
        echo "fuse# must be integer 1 - 50"
        usage
        exit 1
    fi

    if [ $1 -lt 1 -o $1 -gt 50 ]; then
        echo "fuse# out of range"
        usage
        exit 1
    fi

    check_eFuse_driver $1
    eFuse-util $1
else
    usage
    exit 1
fi
