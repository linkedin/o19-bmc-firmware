#!/bin/sh

. /usr/local/bin/openbmc-utils.sh

# enable flash rom CS1 
echo "Enabling flash rom CS1"
devmem_set_bit $(scu_addr 88) 24
