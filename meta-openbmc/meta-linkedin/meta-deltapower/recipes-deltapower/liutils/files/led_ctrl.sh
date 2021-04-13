#!/bin/sh

. /usr/local/bin/ast-functions

usage() {
  echo "Usage: $1 red|green|orange|off"
}

if [ $# != 1 ]; then
   usage
   exit -1
fi

if [ "$1" == "red" ]; then
   echo "Turn LED to red color"
   # set red pin(GPIOP2) high, green pin(GPIOP0) low
   gpio_set P2 1
   gpio_set P0 0
elif [ "$1" == "green" ]; then
   echo "Turn LED to green color"
   gpio_set P2 0
   gpio_set P0 1
elif [ "$1" == "orange" ]; then
   echo "Turn LED to orange color"
   gpio_set P2 1
   gpio_set P0 1
elif [ "$1" == "off" ]; then
   echo "Turn LED off"
   gpio_set P2 0
   gpio_set P0 0
else
   usage
   exit -1
fi
