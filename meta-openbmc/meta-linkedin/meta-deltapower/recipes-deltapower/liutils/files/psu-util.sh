#!/bin/sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin
. /usr/local/bin/ast-functions
. /usr/local/bin/set_control.sh

usage() {
    echo "Usage: $0 [psu_unit [1..6]]" >&2
}

get_i2c_control 12 113
get_i2c_control 1 114
get_i2c_control 2 115
get_i2c_control 3 116
get_i2c_control 6 117
get_i2c_control 7 118

if [ "$#" -ne 1 ]; then
    usage
    exit 1
else
    if [ "$1" == "all" -o "$1" == "ALL" ]; then
        psu_util all
        exit 0
    fi

    if ! [[ "$1" =~ ^[0-9]+$ ]]; then
        echo "PSU# must be integer 1 - 6"
        usage
        exit 1
    fi

    if [ $1 -lt 1 -o $1 -gt 6 ]; then
        echo "PSU unit out of range"
        usage
        exit 1
    else
        # get PSUs info
        check_psu_driver $1 40 psu operation
        output=$(psu_util $1)
        echo "$output"
        exit 0
    fi
fi
