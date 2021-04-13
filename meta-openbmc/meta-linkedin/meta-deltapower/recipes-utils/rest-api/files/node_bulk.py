#!/usr/bin/env python

import sys
import re
from subprocess import *

def getUsageInfo(data, delimiter):
    usage_info = {}

    for sdata in data.split(delimiter):
        tdata = sdata.lstrip().split(' ', 1)
        if (len(tdata) < 2):
            continue

        cpu_usage = float(tdata[0].strip('%'))/100.0
        usage_info[tdata[1].strip()] = cpu_usage

    return usage_info
 
def get_memory_info(data):
    output = {}
    info = {}
    for sdata in data.split('\n'):
        tdata = sdata.split(':', 1)
        if (len(tdata) < 2):
            continue
        memory_in_bytes = int(tdata[1].lstrip().split(" ")[0]) * 1024
        info[tdata[0].strip().lower()] = memory_in_bytes

    output["mem_total"] = info["memtotal"]
    output["mem_available"] = info["memavailable"]
    output["mem_free"] = info["memfree"]
    output["swap_total"] = info["swaptotal"]

    return output

def get_fan_temp_info(fan_temp):
    info = {}

    #Fan 1-4
    for i in range(1, 5):
        fan_info = {}
        temp = ""
        temp_val = fan_temp.split('temperature'+str(i)+':')
        if len(temp_val) > 1:
            temp = float(temp_val[1].lstrip().split(" ")[0])

        speed = ""
        speed_val = fan_temp.split('fan'+ str(i) + ' speed:')
        if len(speed_val) > 1:
            speed = int(speed_val[1].lstrip().split(" ")[0])

        fan_info['temperature'] = temp
        fan_info['speed'] = speed

        data = fan_temp.split('fan_fault:', 1)
        if (len(data) > 1):
            fan_info['fan_fault'] = data[1].split('\n')[0].strip()

        data = fan_temp.split('fan_warning:', 1)
        if (len(data) > 1):
            fan_info['fan_warning'] = data[1].split('\n')[0].strip()

        data = fan_temp.split('fan_status:', 1)
        if (len(data) > 1):
            fan_info['fan_status'] = data[1].split('\n')[0].strip()

        fan_idx = str(i)
        info[fan_idx] = fan_info

    return info
 
def get_efuse_info(out_data, efuse):
    info = {}
    total_vout = 0

    out1 = out_data.split('info for eFuse ')
    for efuse in out1:
        efuse_info = {}
        #get efuse number
        tdata = efuse.split(':', 1)
        if (len(tdata) < 2):
           continue
        efuse_num = str(tdata[0])

        adata = tdata[1].split('\n', 1)
        for sdata in adata[1].split('\n'):
            data = sdata.split(':', 1)
            if (len(data) < 2):
                continue
            if data[0] == "state":
                efuse_info[data[0]] = data[1].strip()
            else: 
                efuse_info[data[0]] = float(data[1].strip())

        info[efuse_num] = efuse_info

    return info

def get_psu_info(out_data, psu):
    aggregate_data = {}
    info = {}

    if psu == "all":
        #split PSU info part and aggregate info part
        sdata = out_data.split('========= PSU Summary =======')
        if (len(sdata) < 2):
             return info

        psu_part = sdata[0]
        aggregate_part = sdata[1].split('\n', 1)[1]
    else:
        psu_part = out_data

    #PSU infor part
    out1 = psu_part.split('info for PSU ')
    for lines in out1:
        psu_info = {}
        temp_info = {}
        tdata = lines.split(':', 1)
        if (len(tdata) < 2):
           continue

        #get the PSU number
        psu_num = str(tdata[0])

        #get PUS info and aggregate info
        for sdata in tdata[1].split("\n"):
            data = sdata.split(':', 1)
            if (len(data) < 2):
                continue

            if "temperature" in data[0]:
                temp_num = data[0].split("temperature")[1]
                temp_info[temp_num] = int(data[1].strip()) 
            elif "operation_state" in data[0]:
                psu_info[data[0].strip()] = data[1].strip()
            else:
                psu_info[data[0].strip()] = float(data[1].strip())

        psu_info["temperature"] = temp_info
        info[psu_num] = psu_info

    #aggregate info
    if psu == "all":
        for sdata in aggregate_part.split('\n'):
            data = sdata.split(':', 1)
            if (len(data) < 2):
                continue
            aggregate_data[data[0]] =  float(data[1])

        aggregate_info = {
            "total_power": aggregate_data["total_power"],
            "side_a_power": aggregate_data["side_a_power"],
            "side_b_power": aggregate_data["side_b_power"],
            "total_current": aggregate_data["total_current"],
        }
        info["aggregate"] = aggregate_info

    return info
