#!/usr/bin/env python

import shlex
from subprocess import *
from node import node
from pal import *
from node_bulk import *

class bulkinfoNode(node):
    def __init__(self, info = None, actions = None):
        if info == None:
            self.info = {}
        else:
            self.info = info

        if actions == None:
            self.actions = []
        else:
            self.actions = actions

    def getInformation(self):
        info = {}
        usage = {}

        #Get all eFuse information
        cmd = '/usr/local/bin/eFuse-util all'
        out_data = Popen(shlex.split(cmd), shell=False, stdout=PIPE).stdout.read().decode()

        efuse_info = get_efuse_info(out_data, "all")

        #Get all PSU information
        cmd = '/usr/local/bin/psu_util all'
        sdata = Popen(shlex.split(cmd), shell=False, stdout=PIPE).stdout.read().decode()
        psu_info = get_psu_info(sdata, "all")

        #get fan information
        cmd = '/usr/local/bin/fan-util.sh'
        data = Popen(shlex.split(cmd), shell=False, stdout=PIPE).stdout.read().decode()
        fan_info = get_fan_temp_info(data)

        #get memory usage
        cmd = 'cat /proc/meminfo'
        data = Popen(shlex.split(cmd), shell=False, stdout=PIPE).stdout.read().decode()
        mem_info = get_memory_info(data)

        #get CPU usage
       # Get Usage information
        data = Popen(shlex.split('top -b n1'), \
                            shell=False, stdout=PIPE).stdout.read().decode()

        cpu_data = data.split('\n')[1].split('CPU:')[1]
        cpu_usage = getUsageInfo(cpu_data, '  ')

        info["efuse"] = efuse_info
        info["psu"] = psu_info
        info["fan"] = fan_info
        usage["cpu"] = cpu_usage
        usage["memory"] = mem_info
        info["system_usage"] = usage
        return info

def get_node_bulkinfo():
    return bulkinfoNode()
