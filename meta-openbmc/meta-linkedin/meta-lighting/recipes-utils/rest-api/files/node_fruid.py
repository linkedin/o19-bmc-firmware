#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *

class fruidNode(node):
    def __init__(self, info = None, actions = None):
        if info == None:
            self.info = {}
        else:
            self.info = info

        if actions == None:
            self.actions = []
        else:
            self.actions = actions

    # Handler for FRUID resource endpoint
    def getInformation(self):
        info = {}
        cmd = '/usr/local/bin/eeprom.sh'
        output = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()

        # need to remove the first info line from eerom.sh command
        adata = output.split('\n', 1)
        for sdata in adata[1].split('\n'):
            tdata = sdata.split(':', 1)
            if (len(tdata) < 2):
                continue
            data_key = tdata[0].strip().lower().replace(" ", "_")
            info[data_key] = tdata[1].strip()

        return info

def get_node_fruid():
    return fruidNode()
