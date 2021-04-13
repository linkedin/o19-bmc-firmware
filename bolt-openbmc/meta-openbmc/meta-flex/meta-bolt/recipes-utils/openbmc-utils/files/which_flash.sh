#!/bin/sh

DEVMEM=/sbin/devmem
WDT2_TMOUT_STAT_REG_OFFSET=0x30
WDT2_TMOUT_STAT_REG_2ND_BOOT_CODE=2

wdt_addr() {
    echo $((0x1E785000 + $1))
}

src_addr=$(wdt_addr $WDT2_TMOUT_STAT_REG_OFFSET)

boot=$($DEVMEM $src_addr)
boot=$((boot&$WDT2_TMOUT_STAT_REG_2ND_BOOT_CODE))

# Check if boot code source is flash1
if [ $boot -eq $WDT2_TMOUT_STAT_REG_2ND_BOOT_CODE ]; then
   logger " !!! BOOTING FALLBACK IMAGE !!!"
   echo " !!! BOOTING FALLBACK IMAGE !!!"
else
   logger " !!! BOOTING PRIMARY IMAGE !!!"
   echo " !!! BOOTING PRIMARY IMAGE !!!"
fi
