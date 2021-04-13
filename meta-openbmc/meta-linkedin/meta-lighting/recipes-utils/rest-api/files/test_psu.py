#!/usr/bin/env python3

import os
from subprocess import *

psu_num = 1
cmd = '/usr/local/bin/psu-eeprom.sh ' + str(psu_num)
sdata = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
#        psu_eeprom = sdata.split(" eeprom:')[1]
psu_eeprom = sdata

#        manuf_name = ""
print (sdata)

manuf_name = psu_eeprom.split("manufactor name: ")
print (len(manuf_name))
print ("manuf_name[0]")
print (manuf_name[0])
print ("manuf_name[1]")
print (manuf_name[1])

if len(manuf_name) > 1:
   manuf_name = manuf_name[1].split('\n')[0]
   manu = manuf_name

   print (manu)


