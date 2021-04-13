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
elif [ "$#" -eq 1 ]; then
    if [ "$1" == "all" -o "$1" == "ALL" ]; then
        echo "Turn on all eFuses:"
        status=0
        END=50
        for i in $(seq 1 $END)
            do
               #50ms delay in between
               usleep 50000
               check_eFuse_driver $i
               dir=$(get_eFuse_dir $i)
               file=$dir/control/shutdown
               echo 0 > $dir/control/shutdown
               if [ ! -f $file ]; then
                   echo "eFuse not found"
                   status=1
                   continue
               fi
               #check if eFuse turned on
               output=$(cat $file)
               state="${output:0:1}"
               if [ "$state" == "0" ]; then
                   echo "Success: Turning on eFuse $i"
               else
                   echo "Failed: Turning on eFuse $i"
                   status=1
               fi
            done

        if [ $status == 0 ]; then
            echo "Success to turn all eFuses on"
        else
            echo "Failed to turn all eFuses on"
        fi
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

    echo "Turning on eFuse $1"
    check_eFuse_driver $1
    dir=$(get_eFuse_dir $1)
    file=$dir/control/shutdown
    echo 0 > $dir/control/shutdown
    if [ ! -f $file ]; then
       echo "eFuse not found"
    fi
    #check if eFuse turned on
    output=$(cat $file)
    state="${output:0:1}"
    if [ "$state" == "0" ]; then
        echo "Success: Turning on eFuse $i"
    else
        echo "Failed: Turning on eFuse $i"
    fi
else
    usage
    exit 1
fi
