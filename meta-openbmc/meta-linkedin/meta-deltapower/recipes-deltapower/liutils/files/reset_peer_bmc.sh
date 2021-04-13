#!/bin/sh

. /usr/local/bin/ast-functions

usage() {
  echo "Usage: $1"
}

if [ $# != 0 ]; then
   usage
   exit -1
fi

echo "Reset peer BMC"
# set pin(GPIOC0) low, and back to high
gpio_set C0 0
usleep 10000
gpio_set C0 1
