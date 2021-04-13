# copyright 2018-present LinkedIn. All Rights Reserved.
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

FUSE_DIR1=/sys/class/i2c-adapter/i2c-0
FUSE_DIR2=/sys/class/i2c-adapter/i2c-4
FUSE_SW1=0x59
FUSE_SW2=0x5a

fuse_addr[0]=0-0012
fuse_addr[1]=0-0013
fuse_addr[2]=0-0010
fuse_addr[3]=0-0011
fuse_addr[4]=0-0046
fuse_addr[5]=0-0047
fuse_addr[6]=0-0045
fuse_addr[7]=0-0044
fuse_addr[8]=0-0043
fuse_addr[9]=0-0042
fuse_addr[10]=0-0041
fuse_addr[11]=0-0040
fuse_addr[12]=0-0056
fuse_addr[13]=0-0057
fuse_addr[14]=0-0054
fuse_addr[15]=0-0055
fuse_addr[16]=0-0052
fuse_addr[17]=0-0053
fuse_addr[18]=0-0051
fuse_addr[19]=0-0050
fuse_addr[20]=0-0017
fuse_addr[21]=0-0016
fuse_addr[22]=0-0015
fuse_addr[23]=0-0014

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
        ADDR=${fuse_addr[$1 - 25]}
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
    echo 0 > $ATTR/control/shutdown
elif [ $2 = "off" ]; then
    echo 1 > $ATTR/control/shutdown
    echo $2
else
    usage
    exit 1
fi
