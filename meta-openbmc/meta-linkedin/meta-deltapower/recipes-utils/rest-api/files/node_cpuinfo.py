#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *

class cpuinfoNode(node):
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
        output = {} 
        p = Popen('/usr/bin/top -b -n 1', stdout=subprocess.PIPE)
        for sdata in data.split('\n'):
            tdata = sdata.split(':', 1)
            if (len(tdata) < 2):
                continue
        
        output[tdata[0].strip()] = tdata[1].strip()
        
        return output
   
def get_node_cpuinfo():
    return cpuinfoNode()
