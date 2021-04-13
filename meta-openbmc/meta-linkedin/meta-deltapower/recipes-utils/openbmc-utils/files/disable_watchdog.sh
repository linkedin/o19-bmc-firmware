#!/bin/sh

. /usr/local/bin/openbmc-utils.sh

# WDT1 Control Register
devmem_clear_bit 0x1e78500c 0
# Disable the dual boot watch dog
devmem_clear_bit 0x1e78502c 0
