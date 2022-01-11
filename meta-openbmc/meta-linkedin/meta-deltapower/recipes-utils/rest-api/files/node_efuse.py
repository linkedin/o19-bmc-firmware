#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *
from node_bulk import *

MIN_EFUSE_NUM = 1
MAX_EFUSE_NUM = 50

class efuseNode(node):
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
            cmd = '/usr/local/bin/eFuse-util ' + str(self.num) + ' --on'
        elif command == 'power-off':
            cmd = '/usr/local/bin/eFuse-util ' + str(self.num) + ' --off'
        elif command == 'reset':
            cmd = '/usr/local/bin/eFuse-util ' + str(self.num) + ' --reset'
        else:
            return -1

        ret = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        if ret.find("Usage:") != -1 or ret.find("fail ") != -1:
           res = 'failed'
        else:
           res = 'success'

        result = { "result": res }
        return result

    def getInformation(self):
        info = {}

        efuse_num = int(self.num)
        if efuse_num >= MIN_EFUSE_NUM and efuse_num <= MAX_EFUSE_NUM:
            cmd = '/usr/local/bin/eFuse-util ' + str(efuse_num)
        else:
            return info

        sdata = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()

        efuse_info = get_efuse_info(sdata, str(self.num))
        return efuse_info

def get_node_efuse(num):
    actions =  ["power-on",
                "reset",
               ]
    return efuseNode(num = num, actions = actions)
