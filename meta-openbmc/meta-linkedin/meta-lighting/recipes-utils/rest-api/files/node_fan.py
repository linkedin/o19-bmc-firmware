#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *
from node_bulk import *

class fanNode(node):
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
        cmd = '/usr/local/bin/fan-util.sh'
        data = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        info = get_fan_temp_info(data)
        return info

def get_node_fan():
    return fanNode()
