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

#FILESEXTRAPATHS_prepend := "${THISDIR}/files/utils:"

SUMMARY = "Flex eeprom Utilities"
DESCRIPTION = "Util for Flex eeprom"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://wemanuf.c;beginline=4;endline=16;md5=da35978751a9d71b73679307c4d296ec"

SRC_URI += "file://utils \
           "

S = "${WORKDIR}/utils"

do_install() {
    localbindir="${D}/usr/local/bin"
    install -d ${localbindir}
#    install -m 0755 wemanuf ${localbindir}/wemanuf
    install -m 644 bmc-id-eeprom.txt ${localbindir}/bmc-ideeprom-txt.txt
    install -m 644 bmc-id-eeprom.example ${localbindir}/bmc-ideeprom-example.example
    install -m 0755 id-eeprom-upg ${localbindir}/id-eeprom-upg
    install -m 0755 id-eeprom-prog ${localbindir}/id-eeprom-prog
    install -m 0755 id-eeprom-prog.py ${localbindir}/id-eeprom-prog.py
    install -m 0755 id-eeprom-show ${localbindir}/id-eeprom-show
    install -m 0755 onie_eep_dbg ${localbindir}/onie_eep_dbg
    install -m 0755 onie-eeprom-prog ${localbindir}/onie-eeprom-prog
    install -m 0755 onie-eeprom-show ${localbindir}/onie-eeprom-show
    install -m 0755 onie-eeprom-upg ${localbindir}/onie-eeprom-upg
    install -m 644 onie-eeprom.txt ${localbindir}/onie-eeprom-txt.txt
    install -m 644 onie-eeprom.example ${localbindir}/onie-eeprom-example.example
}

FILES_${PN} += "/usr/local"
FILES_${PN}-dev = "${libdir}/* ${includedir}"
RDEPENDS_${PN} = "libflex-eeprom"
DEPENDS += "libflex-eeprom"
