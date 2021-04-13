#!/bin/sh

. /usr/local/bin/ast-functions

usage() {
  echo "Usage: $1 red|green"
}

if [ $# != 1 ]; then
   usage
   exit -1
fi

if [ "$1" == "red" ]; then
   echo "Turn LED to red color"
   # set red pin(GPIOC0) high, green pin(GPIOY2) low
   gpio_set C0 1
   gpio_set Y2 0
elif [ "$1" == "green" ]; then
   echo "Turn LED to green color"
   gpio_set C0 0
   gpio_set Y2 1
else
   usage
   exit -1
fi
