#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *

class efuseallNode(node):
    def __init__(self, info = None, actions = None):
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
        if command == 'power-off':
            cmd = '/usr/local/bin/eFuse_off.sh all'
        elif command == 'power-on':
            cmd = '/usr/local/bin/eFuse_on.sh all'
        else:
            return -1

        ret = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        if ret.find("Success") != -1:
           res = 'Sucess - all eFuse turned off'
        else:
           res = 'Some failed to turn on'
        result = { "result": res }
        return result

    def getInformation(self):
        result = {}
        cmd = '/usr/local/bin/eFuse-util.sh all'
        output = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()

        # need to remove the first info line from eFuse-util.sh command
        adata = output.split('\n', 1)
        for sdata in adata[1].split('\n'):
            tdata = sdata.split(':', 1)
            if (len(tdata) < 2):
                continue
            result[tdata[0].strip()] = tdata[1].strip()
        info = {
                "eFuse info:": result,
               }

        return info

def get_node_efuseall():
    actions =  ["power-on",
                "power-off",
               ]
    return efuseallNode()
