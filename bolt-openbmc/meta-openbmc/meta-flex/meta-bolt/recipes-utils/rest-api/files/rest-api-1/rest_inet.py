#!/usr/bin/env python3
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#

import os
from subprocess import *

def get_inet():
    inet_result = []
    mac = {}
    ip = {}
    phy = {}
    link = {}

    from subprocess import check_output
    eth0mac = check_output(['ip','addr','show','eth0'])
    eth0mac = eth0mac.decode()
    eth0lnk = eth0ip = eth0mac

    eth0mac = eth0mac.split("link/ether ")[1].split(" ")[0]
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
#    print x

    flags = len(x)
#    print "flags: %d" % (flags)
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


    fresult = {
                "Inet info": inet_result
              }

    return fresult

if __name__ == '__main__':

    print('{}'.format(get_inet()))

