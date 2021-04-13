#!/bin/sh

/usr/local/bin/eeprom.sh
/usr/local/bin/fan-util.sh

#turn on all eFuses
for i in `seq 1 50`;
    do
       echo "Turning off eFuse $i"
       /usr/local/bin/eFuse_off.sh $i
    done

/usr/local/bin/eFuse-util.sh all

#turn off all eFuses
for i in `seq 1 50`;
    do
       echo "Turning on eFuse $i"
       /usr/local/bin/eFuse_on.sh $i
    done

/usr/local/bin/eFuse-util.sh all

#test turn on/off all eFuses
/usr/local/bin/eFuse_off.sh all
/usr/local/bin/eFuse-util.sh all
/usr/local/bin/eFuse_on.sh all
/usr/local/bin/eFuse-util.sh all

#PSU
for i in `seq 1 6`;
    do
       echo "Turning on PSU $i"
       psu_off.sh $i
    done

for i in `seq 1 6`;
    do
       psu-util.sh $i
    done

for i in `seq 1 6`;
    do
       echo "Turning on PSU $i"
       psu_on.sh $i
    done

for i in `seq 1 6`;
    do
       psu-util.sh $i
    done

#PSU eeprom
 for i in `seq 1 6`;
    do
       psu-eeprom.sh $i
       psu-util.sh $i
    done

/usr/local/bin/get_sw_version.sh
/usr/local/bin/get_bmc_id.sh
