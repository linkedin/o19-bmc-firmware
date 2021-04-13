#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *

class meminfoNode(node):
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
        cmd = 'cat /proc/meminfo'
        data = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        info = get_memory_info(data)
        return info

def get_node_meminfo():
    return meminfoNode()
