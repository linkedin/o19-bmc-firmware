#!/usr/bin/env python

from subprocess import *
from node import node
from pal import *

class inetNode(node):
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
        inet_result = []
        mac = {}
        ip = {}
        phy = {}
        link = {}

        cmd = 'ip addr show eth0'
        data = Popen(cmd, shell=True, stdout=PIPE).stdout.read().decode()
        eth0lnk = eth0ip = data

        eth0mac = data.split("link/ether ")[1].split(" ")[0]
        mac["BMC MAC"] = eth0mac
        inet_result.append(mac)

        eth0ip = eth0ip.split("inet ")
        if len(eth0ip) > 1:
            eth0ip = eth0ip[1].split("/")[0]
            if eth0ip:
               ip["BMC IP"] = eth0ip
            else:
               ip["BMC IP"] = 'UNDEFINED'
        else:
            ip["BMC IP"] = 'UNDEFINED'
        inet_result.append(ip)

        ipv6 = {}
        eth0ip = eth0lnk.split("inet6 ")
        if len(eth0ip) > 1:
           scopes = {}
           for addr in eth0ip[1:]:
              addr = addr.split(" scope ")
              scope = addr[1].split(" ")
              scopes.setdefault(scope[0].upper(),[]).append(addr[0])
           for scope in scopes:
              ipv6["BMC IPV6 " + scope] = scopes[scope]
        else:
            ipv6["BMC IPV6"] = 'UNDEFINED'
        inet_result.append(ipv6)

        link["LINK"] = "DOWN"
        phy["PHY"] = "DOWN"

        eth0lnk = eth0lnk.split("eth0: <")[1]
        eth0lnk = eth0lnk.split(">")[0]

        x = eth0lnk.split(',')
        flags = len(x)
        if flags > 1:
            i = 0
            while i < flags:
                if x[i] == 'UP':
                    link["LINK"] = "UP"
                elif x[i] == "LOWER_UP":
                    phy["PHY"] = 'UP'
                i += 1

        inet_result.append(link)
        inet_result.append(phy)

        info = {
                "inet info": inet_result,
               }
        return info

def get_node_inet():
    return inetNode()
