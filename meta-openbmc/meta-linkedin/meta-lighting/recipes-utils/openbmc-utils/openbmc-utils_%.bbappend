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

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://disable_watchdog.sh \
            file://enable_watchdog_ext_signal.sh \
            file://ast-functions \
            file://ec_version.sh \
            file://board-utils.sh \
            file://setup_board.sh \
            file://set_control.sh \
            file://setup_i2c.sh \
            file://setup_efuse.sh \
            file://setup-gpio.sh \
            file://get_bmc_id.sh \
            file://set_bmc_id.sh \
            file://eth0_mac_fixup0.sh \
            file://configure_host.sh \
            file://dhcp_renew.sh \
            file://reimage.py \
            file://get_ip.sh \
           "

OPENBMC_UTILS_FILES += " \
    disable_watchdog.sh \
    enable_watchdog_ext_signal.sh \
    ast-functions \
    set_control.sh \
    ec_version.sh \
    get_bmc_id.sh \
    set_bmc_id.sh \
    eth0_mac_fixup0.sh \
    setup_i2c.sh \
    setup_efuse.sh \
    setup-gpio.sh \
    configure_host.sh \
    dhcp_renew.sh \
    reimage.py \
    get_ip.sh \
    "

DEPENDS_append = " update-rc.d-native"
RDEPENDS_${PN} += "python3-core"

do_install_board() {
   # for backward compatible, create /usr/local/fbpackages/utils/ast-functions
   olddir="/usr/local/fbpackages/utils"
   install -d ${D}${olddir}
   ln -s "/usr/local/bin/openbmc-utils.sh" ${D}${olddir}/ast-functions

    # init
    install -d ${D}${sysconfdir}/init.d
    install -d ${D}${sysconfdir}/rcS.d
    install -m 0755 ast-functions ${D}${sysconfdir}/ast-functions
    install -m 0755 set_control.sh ${D}${sysconfdir}/set_control.sh
    install -m 0755 get_bmc_id.sh ${D}${sysconfdir}/get_bmc_id.sh
    install -m 0755 dhcp_renew.sh ${D}${sysconfdir}/dhcp_renew.sh
    install -m 0755 reimage.py ${D}${sysconfdir}/reimage.py
    # the script to mount /mnt/data
    install -m 0755 ${WORKDIR}/mount_data0.sh ${D}${sysconfdir}/init.d/mount_data0.sh
    update-rc.d -r ${D} mount_data0.sh start 03 S .
    install -m 0755 ${WORKDIR}/rc.early ${D}${sysconfdir}/init.d/rc.early
    update-rc.d -r ${D} rc.early start 04 S .

    install -m 755 setup_i2c.sh ${D}${sysconfdir}/init.d/setup_i2c.sh
    update-rc.d -r ${D} setup_i2c.sh start 60 S .

    install -m 755 setup_efuse.sh ${D}${sysconfdir}/init.d/setup_efuse.sh
    update-rc.d -r ${D} setup_efuse.sh start 60 5 .

    # networking is done after rcS, any start level within rcS
    # for mac fixup should work
    install -m 755 eth0_mac_fixup0.sh ${D}${sysconfdir}/init.d/eth0_mac_fixup0.sh
    update-rc.d -r ${D} eth0_mac_fixup0.sh start 70 S .

    install -m 755 setup_board.sh ${D}${sysconfdir}/init.d/setup_board.sh
    update-rc.d -r ${D} setup_board.sh start 80 S .

    install -m 0755 ${WORKDIR}/rc.local ${D}${sysconfdir}/init.d/rc.local
    update-rc.d -r ${D} rc.local start 99 2 3 4 5 .

    install -m 0755 ${WORKDIR}/disable_watchdog.sh ${D}${sysconfdir}/init.d/disable_watchdog.sh
    update-rc.d -r ${D} disable_watchdog.sh start 99 2 3 4 5 .

    install -m 0755 ${WORKDIR}/enable_watchdog_ext_signal.sh ${D}${sysconfdir}/init.d/enable_watchdog_ext_signal.sh
    update-rc.d -r ${D} enable_watchdog_ext_signal.sh start 99 2 3 4 5 .

    install -m 755 setup-gpio.sh ${D}${sysconfdir}/init.d/setup-gpio.sh
    update-rc.d -r ${D} setup-gpio.sh start 59 5 .

    install -m 755 configure_host.sh ${D}${sysconfdir}/init.d/configure_host.sh
    update-rc.d -r ${D} configure_host.sh start 20 5 .

    install -m 755 get_ip.sh ${D}${sysconfdir}/init.d/get_ip.sh
    update-rc.d -r ${D} get_ip.sh start 80 5 .
}

do_install_append() {
  do_install_board
}

FILES_${PN} += "${sysconfdir}"
