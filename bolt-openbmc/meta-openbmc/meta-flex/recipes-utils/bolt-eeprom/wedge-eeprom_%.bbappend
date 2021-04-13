# Copyright 2016-present Flex. All Rights Reserved.
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
# File: bolt-eeprom_%.bbappend
#
# Author: Tiju Jacob (Tiju.Jacob@flextronics.com)
#

FILESEXTRAPATHS_prepend := "${THISDIR}/files/utils:"

SRC_URI += "file://0002-Adding-eeprom-utilities-to-Makefile.patch \
            file://0003-Modify-weutil.c-for-LI.patch \
            file://0004-Create-bmc-id-eeprom.txt.patch \
            file://0005-Create-wemanuf.c.patch \
            file://0006-Create-id-eeprom-upg.c.patch \
            file://0007-New-UTS-friendly-utils-for-ID-eeprom.patch \
            file://0008-Create-onie-eep-dbg.c.patch \
            file://0009-UTS-friendly-ONIE-eeprom-utilities.patch \
           "

do_install_append() {
    install -m 0755 wemanuf ${D}${bindir}/wemanuf
    install -m 644 bmc-id-eeprom.txt ${D}${bindir}/bmc-id-eeprom.txt
    install -m 444 bmc-id-eeprom.example ${D}${bindir}/bmc-id-eeprom.example
    install -m 0755 id-eeprom-upg ${D}${bindir}/id-eeprom-upg
    install -m 0755 id-eeprom-prog ${D}${bindir}/id-eeprom-prog
    install -m 0755 id-eeprom-show ${D}${bindir}/id-eeprom-show
    install -m 0755 onie_eep_dbg ${D}${bindir}/onie_eep_dbg
    install -m 0755 onie-eeprom-prog ${D}${bindir}/onie-eeprom-prog
    install -m 0755 onie-eeprom-show ${D}${bindir}/onie-eeprom-show
    install -m 0755 onie-eeprom-upg ${D}${bindir}/onie-eeprom-upg
    install -m 644 onie-eeprom.txt ${D}${bindir}/onie-eeprom.txt
    install -m 644 onie-eeprom.example ${D}${bindir}/onie-eeprom.example
}
