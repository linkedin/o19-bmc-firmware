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

usage() {
    echo "Usage: $0 <on|off> <SW|LUN|ALL-LUNS|ALL> <HW|HWDH|FW|FWDH> <LUN#>" >&2
    echo "    Options:  on|off:          Turning on/off Switch or LUN"
    echo "              SW|LUN|ALL-LUNS|ALL:          Switch, LUN, ALL-LUNS, ALL(LUNS and Switch)"
    echo "              HW|HWDH|FW|FWDH: Half Width, Half Width Doule Height, Full width, Full Width Double Height"
    echo "              <LUN#>: 1 - 48 "
}

#LUN to eFuse mapping
lun_2_efuse=( 24 48 23 47 22 46 21 45 20 44 19 43 18 42 17 41 16 40 15 39 \
              14 38 13 37  1 25  2 26  3 27  4 28  5 29  6 30  7 31  8 32 \
               9 33 10 34 11 35 12 36 24 48 23 47 22 46 21 45 20 44 19 43 \
              18 42 17 41 16 40 15 39 14 38 13 37 )

set +e

if [ "$#" -lt 2 ]; then
    usage
    exit 1
fi

option1=$(echo $1 | tr [a-z] [A-Z])
option2=$(echo $2 | tr [a-z] [A-Z])

echo "option1: $option1"

if [ $option1 != "ON" ] && [ $option1 != "OFF" ]; then
    echo "invalid option: $option1"
    usage
    exit 1
fi

if [ "$#" -eq 2 ]; then
    if [ $option2 = "SW" ]; then
        echo "Switch"
        echo "Turning $option1 eFuse49 and eFuse50"
        efuses=(49 50)
    elif [ $option2 = "ALL-LUNS" ]; then
        echo "Turning $option1 all LUNs"
        efuses=( $(seq 1 48 ) )
    elif [ $option2 = "ALL" ]; then
        echo "Turning $option1 all LUNs and switch"
        efuses=( $(seq 1 50 ) )
    else
        usage
        exit 1
    fi
elif [ "$#" -eq 3 ]; then
    usage
    exit 1
elif [ "$#" -eq 4 ]; then
    option3=$(echo $3 | tr [a-z] [A-Z])
    echo "option3: $option3"
    #LUN is between 1 - 48
    lun=$4
    echo "option4: LUN=$lun"
    #LUN number must be integer
    if ! [[ "$4" =~ ^[0-9]+$ ]]; then
        echo "LUN# must be integer 1 - 48"
        usage
        exit 1
    fi

    if [ $4 -lt 1 -o $4 -gt 48 ]; then
        echo "LUN# out of range of 1-48"
        usage
        exit 1
    fi

    case $option3 in
       HW)
          echo "Half width"
          eFuse1=${lun_2_efuse[$lun-1]}
          echo "Turning on LUN$lun eFuse: $eFuse1"
          efuses=($eFuse1)
          ;;
       HWDH)
          echo "Half width Double height"
          #LUN should be 1,2, 5,6, 9, 10 ...
          if [ $((lun%4)) != 1 ] && [ $((lun%4)) != 2 ]; then
             echo "Invalid LUN number:  LUN# should be 1,2,5,6 ..."
             usage
             exit
          fi

          eFuse1=${lun_2_efuse[$lun-1]}
          eFuse2=${lun_2_efuse[$lun+1]}
          echo "Turning off LUN$lun eFuses: $eFuse1 and $eFuse2"
          #efuses
          efuses=($eFuse1 $eFuse2)
          ;;
       FW)
          echo "Full width"
          #lun should be odd number
          if [ $((lun%2)) -eq 0 ]; then
             echo "LUN number should odd number lun$lun"
             usage
             exit 1
          else
             echo "odd lun$lun 2 eFuses";
             eFuse1=${lun_2_efuse[$lun-1]}
             eFuse2=${lun_2_efuse[$lun]}
             echo "Turning off LUN$lun eFuses: $eFuse1 and $eFuse2"
             #efuse util goes here
             efuses=($eFuse1 $eFuse2)
          fi
          ;;
       FWDH)
          echo "Full width Double height"
          #LUN should be 1, 5, 9, 13 ...
          if [ $((lun%4)) != 1 ]; then
             echo "Invalid LUN number:  LUN# should be 1, 5, 9, 13 ..."
             usage
             exit 1
          fi

          eFuse1=${lun_2_efuse[lun-1]}
          eFuse2=${lun_2_efuse[lun]}
          eFuse3=${lun_2_efuse[lun+1]}
          eFuse4=${lun_2_efuse[lun+2]}
          echo "Turning off LUN$lun eFuses: $eFuse1 $eFuse2 $eFuse3, $eFuse4"
          #efuse util goes here
          efuses=($eFuse1 $eFuse2 $eFuse3 $eFuse4)
          ;;
       *)
          usage
          exit 1
          ;;
    esac
fi

echo "eFuse actions:"
for i in "${efuses[@]}"
   do
      if [ $option1 == "ON" ]; then
         echo "Turn on eFuse $i"
         eFuse_on.sh $i
      else
         echo "Turn off eFuse $i"
         eFuse_off.sh $i
      fi
   done
