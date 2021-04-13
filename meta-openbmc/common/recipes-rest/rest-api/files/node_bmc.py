#!/usr/bin/env python
#
# Copyright 2015-present Facebook. All Rights Reserved.
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

from subprocess import *
import re
from node import node
from pal import *
from uuid import getnode as get_mac
import os.path

def getSPIVendor(manufacturer_id):
    # Define Manufacturer ID
    MFID_WINBOND = "EF"    # Winbond
    MFID_MICRON = "20"     # Micron
    MFID_MACRONIX = "C2"   # Macronix

    vendor_name = {
        MFID_WINBOND: "Winbond",
        MFID_MICRON: "Micron",
        MFID_MACRONIX: "Macronix",
        }

    if manufacturer_id in vendor_name:
        return(vendor_name[manufacturer_id])
    else:
        return("Unknown")

def get_mac_addr():
    mac_addr = ""
    # Get MAC Address
    mac_path = "/sys/class/net/eth0/address"
    if os.path.isfile(mac_path):
        mac = open('/sys/class/net/eth0/address').read()
        mac_addr = mac[0:17].upper()
    else:
        mac = get_mac()

        if mac:
            mac_addr = ':'.join(("%012X" % mac)[i:i+2] for i in range(0, 12, 2))

    return mac_addr

def get_reset_reason(wdt_counter):
    wdt_counter = int(wdt_counter, 0)
    wdt_counter &= 0xff00

    if wdt_counter:
        reset_reason = "User Initiated Reset or WDT Reset"
    else:
        reset_reason = "Power ON Reset"

    return reset_reason

def getUsageInfo(data, delimiter):
    usage_info = {}

    for sdata in data.split(delimiter):
        tdata = sdata.lstrip().split(' ', 1)
        if (len(tdata) < 2):
            continue

        cpu_usage = float(tdata[0].strip('%'))/100.0
        usage_info[tdata[1].strip()] = cpu_usage

    return usage_info

class bmcNode(node):
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
        # Get Platform Name
        name = pal_get_platform_name().decode()

        # Get MAC Address
        mac_addr = get_mac_addr()

        # Get BMC Reset Reason
        wdt_counter = Popen('devmem 0x1e785010', \
                            shell=True, stdout=PIPE).stdout.read().decode()
        reset_reason = get_reset_reason(wdt_counter)

        # Get BMC's Up Time
        data = Popen('uptime', \
                        shell=True, stdout=PIPE).stdout.read().decode()
        uptime = data.strip()

        # Get Usage information
        data = Popen('top -b n1', \
                            shell=True, stdout=PIPE).stdout.read().decode()

        cpu_usage = {}
        tdata = data.split('CPU:')
        if (len(tdata) > 1):
            cpu_data = tdata[1].split("\n")[0]
            cpu_usage = getUsageInfo(cpu_data, '  ')

        # Get OpenBMC version
        obc_version = ""
        data = Popen('cat /etc/issue', \
                            shell=True, stdout=PIPE).stdout.read().decode()

        # OpenBMC Version
        ver = re.search(r'[v|V]([\w\d._-]*)\s', data)
        if ver:
            obc_version = ver.group(1)

        # Get U-boot Version
        uboot_version = ""
        data = Popen( 'strings /dev/mtd0 | grep U-Boot | grep 20', \
                            shell=True, stdout=PIPE).stdout.read().decode()

        # U-boot Version
        lines=data.splitlines()
        data_len=len(lines)

        for i in range(data_len):
            if i!=data_len-1:
                uboot_version += lines[i] +", "
            else:
                uboot_version += lines[i]

        # Get kernel release and kernel version
        kernel_release = ""
        data = Popen( 'uname -r', \
                            shell=True, stdout=PIPE).stdout.read().decode()
        kernel_release = data.strip('\n')

        kernel_version = ""
        data = Popen( 'uname -v', \
                            shell=True, stdout=PIPE).stdout.read().decode()
        kernel_version = data.strip('\n')

        # SPI0 Vendor
        spi0_vendor = ""
        data = Popen('cat /tmp/spi0.0_vendor.dat | cut -c1-2', \
                            shell=True, stdout=PIPE).stdout.read().decode()
        spi0_mfid = data.strip('\n')
        spi0_vendor = getSPIVendor(spi0_mfid)

        # SPI1 Vendor
        spi1_vendor = ""
        data = Popen('cat /tmp/spi0.1_vendor.dat | cut -c1-2', \
                            shell=True, stdout=PIPE).stdout.read().decode()
        spi1_mfid = data.strip('\n')
        spi1_vendor = getSPIVendor(spi1_mfid)

        info = {
            "description": name + " BMC",
            "mac_address": mac_addr,
            "reset_reason": reset_reason,
            "uptime": uptime,
            "cpu_usage": cpu_usage,
            "openbmc_version": obc_version,
            "u-boot_version": uboot_version,
            "kernel_version": kernel_release + " " + kernel_version,
            "spi0_vendor": spi0_vendor,
            "spi1_vendor": spi1_vendor,
            }

        return info

    def doAction(self, data):
        result = 'success'
        if (data["action"] == 'reboot'):
            Popen('sleep 1; /sbin/reboot', shell=True, stdout=PIPE)
            result = 'success'
        elif (data["action"] == 'update_password'):
            passwd=data['password']
            if ('user' in data):
                user=data['user']
            else:
                user='root'
            pal_update_password(user, passwd)
            result = 'success'
        result = {"result": result}

        return result

def get_node_bmc():
    actions = ["reboot", "update_password"]
    return bmcNode(actions = actions)
