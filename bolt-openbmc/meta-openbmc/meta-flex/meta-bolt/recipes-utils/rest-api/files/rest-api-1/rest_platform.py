#!/usr/bin/env python3

import re
import subprocess
import bmc_command

# Handler for sensors resource endpoint
def get_platform():
    sresult={}
    proc = subprocess.Popen(['/usr/local/bin/id-eeprom-show'],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    try:
        data, err = bmc_command.timed_communicate(proc)
        data = data.decode()
    except bmc_command.TimeoutError as ex:
        data = ex.output
        err = ex.error

    data = re.sub(r'\(.+?\)', '', data)
    for adata in data.split('\n'):
        if(len(adata) < 2):
                break;

        sdata = adata.split (':', 1)

        if (len(sdata) < 2):
            continue
        sresult[sdata[0].strip()] = sdata[1].strip()
    result=[sresult]
    fresult = {
        "PlatformState": result
        }
    return fresult

if(__name__=="__main__"):
    print('{}'.format(get_platform()))
