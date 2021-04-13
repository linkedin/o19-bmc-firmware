#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *

class swverNode(node):
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
        swVersion=" "
        bmc_id = " "
        cmd = '/usr/local/bin/get_sw_version.sh'
        result = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        swVersion = result.strip()
        cmd = '/usr/local/bin/get_bmc_id.sh'
        result = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        bmc_id = result.strip()

        info = {
                "BMC software Version": swVersion,
                "BMC identity": bmc_id,
               }

        return info

def get_node_swver():
    return swverNode()
