#!/bin/sh

. /usr/local/bin/ast-functions

usage() {
  echo "Usage: gpio-util --get GPIO#"
  echo "       gpio-util --set GPIO# 0|1"
  exit -1
}

if [ $# = 0 ]; then
   usage
   exit -1
fi

#num=$(gpio_name2value $2)
#dir=$(gpio_dir $num)
#direction=$(cat $dir/direction)

#echo "GPIO num: $num  direction: $direction"
#cat $dir/direction

if [ "$1" = "--get" ]; then
   if [ "$#" != 2 ]; then
      usage
   else
      val=$(gpio_get $2)
   echo $val
   fi
elif [ "$1" = "--set" ]; then
   if [ "$#" != 3 ]; then
       usage
       exit -1
   else
       val=$(gpio_get $2)
       echo "GPIO$2 is $val"
       gpio_set $2 $3
       val=$(gpio_get $2)
       echo "GPIO$2 is set to $val"
   fi
else
  echo "Usage: gpio-util --get GPIO#"
  echo "       gpio-util --set GPIO#"
  exit -1
fi

num=$(gpio_name2value $2)
dir=$(gpio_dir $num)
direction=$(cat $dir/direction)
