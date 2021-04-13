# copyright 2015-present Facebook. All Rights Reserved.
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

FANCTRL_DIR=/sys/class/i2c-adapter/i2c-8/8-0030/fanctrl

usage() {
    echo "Usage: $0 [eFuse (1..50)] on/off" >&2
}

    if [ $1 -gt 50 ]; then
        usage
        exit 1
    elif [ $1 -eq 49 ]; then
        FUSE_DIR=$FUSE_DIR2
        ADDR=$FUSE_SW1
    elif [ $1 -eq 50 ]; then
        FUSE_DIR=$FUSE_DIR2
        ADDR=$FUSE_SW2
    elif [ $1 -gt 24 ]; then
        FUSE_DIR=$FUSE_DIR2
        ADDR=$fuse_addr[$1 - 25]
    else
        FUSE_DIR=$FUSE_DIR1
        ADDR=${fuse_addr[$1 - 1]}

    fi

    ATTR=$FUSE_DIR/$ADDR

#set -e

if [ "$#" -le 1 ]; then
    usage
    exit 1
elif [ $1 -gt 50 ]; then
        usage
        exit 1
fi
if [ $2 = "on" ]; then
    echo $2
#    echo 1 > $ATTR/control/shutdown
    echo $ATTR
elif [ $2 = "off"]; then
#    echo 0 > $ATTR/control/shutdown 
    echo $ATTR
    echo $2
else
    usage
    exit 1
fi
