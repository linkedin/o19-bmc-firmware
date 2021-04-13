#!/bin/sh

if [ "$1" == "slot1" ]; then
   channel=1
elif [ "$1" == "slot2" ]; then
   channel=2
elif [ "$1" == "slot3" ]; then
   channel=4
elif [ "$1" == "slot4" ]; then
   channel=8
else
  echo "Usage: poweroff_cpu.sh [ slot1 | slot2 | slot3 | slot4 ]"
  exit -1
fi

I2C_BUS=9

# Power off
echo "Making power button high"
i2cset -f -y $I2C_BUS 0x77 $channel
i2cset -f -y $I2C_BUS 0x31 0x30 0x11   # making power button high by writing to CPLD register 0x30
usleep 1000000

echo "Making power button low"
i2cset -f -y $I2C_BUS 0x31 0x30 0x10  # making power button low by writing to CPLD register 0x30
usleep 10000000

echo "Making power button high"
i2cset -f -y $I2C_BUS 0x31 0x30 0x11   # making power button high by writing to CPLD register 0x30
usleep 1000000


