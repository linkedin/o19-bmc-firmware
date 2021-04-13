#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *
from node_bulk import *

class psuNode(node):
    def __init__(self, num = None, info = None, actions = None):
        self.num = num

        if info == None:
            self.info = {}
        else:
            self.info = info

        if actions == None:
            self.actions = []
        else:
            self.actions = actions

    def doAction(self, data):
        command = data["action"]
        if command == 'power-on':
            cmd = '/usr/local/bin/psu_on.sh ' + str(self.num)
        else:
            return -1

        ret = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        if ret.find("Usage:") != -1 or ret.find("fail ") != -1:
           res = 'failed'
        elif ret.find("Turning on") != -1:
           res = 'success'
        else:
           res = 'not a valid action'

        result = { "result": res }
        return result

    def getInformation(self):
        min_psu_num = 1
        max_psu_num = 6
        info = {}

        if self.num >= min_psu_num and self.num <= max_psu_num:
            cmd = '/usr/local/bin/psu_util ' + str(self.num)
        else:
            return info

        output = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        info = get_psu_info(output, str(self.num))
        return info
 
def get_node_psu(num):
    actions =  ["power-on",
               ]
    return psuNode(num = num, actions = actions)
