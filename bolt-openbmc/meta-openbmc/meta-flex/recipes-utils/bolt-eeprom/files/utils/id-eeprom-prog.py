#!/usr/bin/python
  
import os

eeprom_list = ["Magic Word",
               "Version",
               "Product Name",
               "Product Part Number",
               "System Assembly Part Number",
               "LinkedIn PCBA Part Number",
               "LinkedIn PCB Part Number",
               "ODM PCBA Part Number",
               "ODM PCBA Serial Number",
               "Product Production State",
               "Product Version",
               "Product Sub-Version",
               "Product Serial Number",
               "Product Asset Tag",
               "System Manufacturer",
               "System Manufacturing Date",
               "PCB Manufacturer",
               "Assembled At",
               "Local MAC",
               "Extended MAC Base",
               "Extended MAC Address Size",
               "Reserved",
               "CRC8"]

#Linkedin V2
#eeprom_size = [4, 2, 16, 16, 12, 14, 14, 14, 24, 2, 1, 1, 28, 12, 8, 8, 8, 8, 12, 12, 2, 8, 1]

#LinkedIn V1
#eeprom_size = [4, 2, 16, 16, 12, 14, 14, 14, 12, 2, 1, 1, 16, 12, 8, 8, 8, 8, 12, 12, 2, 8, 1]

#LinkedIn V0
eeprom_size = [2, 15, 11, 11, 12, 12, 13, 13, 1, 1, 1, 12, 13, 5, 4, 5, 3, 12, 12, 2, 8, 1]

idfile="/sys/class/i2c-adapter/i2c-8/8-0050/eeprom"
version = 0

def crc8(buff):
    crc = 0
    for b in buff:
        crc ^= b
        for i in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0x8C
            else:
                crc >>= 1
    return crc

def read_eeprom_file():
    output=""
    with open("/usr/local/bin/bmc-id-eeprom1.txt", 'r') as fin:
       data=fin.read()
       print(data)
       fin.close()

    for i in range(len(eeprom_list)):
       key = eeprom_list[i] + ":"
       tdata = data.split(key)[1].split("\n")[0]
       if (len(tdata) < eeprom_size[i]):                      
           tdata+=" "*(eeprom_size[i]-len(tdata))             
       else:                                                  
           #truncate the field to the specific length         
           tdata = tdata[0:(eeprom_size[i])]

       output+=tdata
    return output

def write_eeprom(data):
   with open(idfile, 'w') as fout:
      fout.write(data)
      fout.close()

if __name__ == "__main__":
   outdata=read_eeprom_file()
   if (version == 0):
       write_eeprom(outdata)
   elif (version == 1):
       slist= list(outdata)
       #verion 1 integer
       slist[4] = chr(0)
       slist[5] = chr(1)
       #mac size: integer
       #   slist[-10] = chr(0)
       #   slist[-11] = chr(0)
       #   crc_list = slist[:-1]
       #   length = len(slist)
       #   print("len: {}".format(length))
       #   crc_val = crc8(crc_list)
       #   print("CRC8: {}".format(crc_val))
       #   slist[-1] = chr(crc_val)
       out_write = "".join(slist)
       print(out_write)
       write_eeprom(out_write)

