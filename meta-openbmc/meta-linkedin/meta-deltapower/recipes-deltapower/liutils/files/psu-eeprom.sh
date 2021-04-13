#!/bin/sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin
. /usr/local/bin/set_control.sh
. /usr/local/bin/ast-functions

usage() {
    echo "Usage: $0 [psu unit [1..6]]" >&2
}

get_i2c_control 12 113
get_i2c_control 1 114
get_i2c_control 2 115
get_i2c_control 3 116
get_i2c_control 6 117
get_i2c_control 7 118

if [ "$#" -eq 0 ]; then
    echo "please input PSU#"
    exit 1
elif [ "$#" -eq 1 ]; then
    if ! [[ "$1" =~ ^[0-9]+$ ]]; then
        echo "psu# must be an integer 1 - 6"
        usage
        exit 1
    fi

    if [ $1 -lt 1 -o $1 -gt 6 ]; then
        echo "PSU unit out of range"
        usage
        exit 1
    else
        # get PSUs info
        check_psu_driver $1 57 24c64 eeprom
        output=$(psu-eeprom $1)
        echo "$output"
    fi
else
    usage
    exit 1
fi

