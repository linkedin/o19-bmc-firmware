#!/bin/sh

if [ "$1" == "slot1" ] || [ "$1" == "slot2" ] || [ "$1" == "slot3" ] || [ "$1" == "slot4" ]
then
  SLOT=$1
else
  echo "Usage: sol-util [ slot1 | slot2 | slot3 | slot4 ]"
  exit -1
fi

if [ "$1" == "slot1" ]; then
   channel=1
elif [ "$1" == "slot2" ]; then
   channel=2
elif [ "$1" == "slot3" ]; then
   channel=4
elif [ "$1" == "slot4" ]; then
   channel=8
fi

I2C_BUS=9

# Power ON
i2cset -f -y $I2C_BUS 0x77 $channel   # open SoC CPLD I2C
echo "making power button high"
i2cset -f -y $I2C_BUS 0x31 0x30 0x11  # making power button high by writing to CPLD register 0x30
usleep 1000000

echo "making power button low"
i2cset -f -y $I2C_BUS 0x31 0x30 0x10  # making power button low by writing to CPLD register 0x30
#usleep 100000
usleep 1000000

echo "making power button high"
i2cset -f -y $I2C_BUS 0x31 0x30 0x11  # making power button high by writing to CPLD register 0x30
usleep 1000000

