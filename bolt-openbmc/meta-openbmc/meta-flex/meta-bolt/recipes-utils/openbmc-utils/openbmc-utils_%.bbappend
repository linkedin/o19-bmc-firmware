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
            file://ec_version.sh \
            file://board-utils.sh \
            file://setup_board.sh \
            file://cpld_rev.sh \
            file://cpld_upgrade.sh \
            file://cp2112_i2c_flush.sh \
            file://reset_qsfp_mux.sh \
            file://setup_i2c.sh \
            file://tunnel_setup.sh \
            file://remote_cpu_access.sh \
            file://lsb_release \
            file://0001-Bolt-iso-buffer-enabled.patch \
            file://bolt_reset.sh \
            file://bolt_meminfo \
            file://bolt_cpuinfo \
            file://bolt_SYSCPLD_ver0p6.jbc \
            file://0002-Change-Local-MAC-to-BMC-MAC.patch \
            file://0003-Enable-access-to-ONIE-eeprom.patch \
            file://enable_flash_cs1.sh \
            file://which_flash.sh \
            file://flash-upg \
            file://clear-boot-sel \
            file://boot0 \
           "

OPENBMC_UTILS_FILES += " \
    disable_watchdog.sh \
    enable_watchdog_ext_signal.sh \
    cpld_upgrade.sh \
    cpld_rev.sh \
    ec_version.sh \
    cp2112_i2c_flush.sh \
    reset_qsfp_mux.sh \
    lsb_release \
    bolt_reset.sh \
    bolt_meminfo \
    bolt_cpuinfo \
    bolt_SYSCPLD_ver0p6.jbc \
    enable_flash_cs1.sh \
    which_flash.sh \
    flash-upg \
    clear-boot-sel \
    boot0 \
    "

DEPENDS_append = " update-rc.d-native"

do_install_board() {
    # for backward compatible, create /usr/local/fbpackages/utils/ast-functions
    olddir="/usr/local/fbpackages/utils"
    install -d ${D}${olddir}
    ln -s "/usr/local/bin/openbmc-utils.sh" "${D}${olddir}/ast-functions"

    # init
    install -d ${D}${sysconfdir}/init.d
    install -d ${D}${sysconfdir}/rcS.d
    # the script to mount /mnt/data
    install -m 0755 ${WORKDIR}/mount_data0.sh ${D}${sysconfdir}/init.d/mount_data0.sh
    update-rc.d -r ${D} mount_data0.sh start 03 S .
    install -m 0755 ${WORKDIR}/rc.early ${D}${sysconfdir}/init.d/rc.early
    update-rc.d -r ${D} rc.early start 04 S .

    install -m 755 setup_i2c.sh ${D}${sysconfdir}/init.d/setup_i2c.sh
    update-rc.d -r ${D} setup_i2c.sh start 60 S .

    # networking is done after rcS, any start level within rcS
    # for mac fixup should work
    install -m 755 eth0_mac_fixup.sh ${D}${sysconfdir}/init.d/eth0_mac_fixup.sh
    update-rc.d -r ${D} eth0_mac_fixup.sh start 70 S .

    install -m 755 setup_board.sh ${D}${sysconfdir}/init.d/setup_board.sh
    update-rc.d -r ${D} setup_board.sh start 80 S .

    install -m 755 power-on.sh ${D}${sysconfdir}/init.d/power-on.sh
    update-rc.d -r ${D} power-on.sh start 85 S .

    install -m 0755 ${WORKDIR}/rc.local ${D}${sysconfdir}/init.d/rc.local
    update-rc.d -r ${D} rc.local start 99 2 3 4 5 .

    install -m 0755 ${WORKDIR}/disable_watchdog.sh ${D}${sysconfdir}/init.d/disable_watchdog.sh
    update-rc.d -r ${D} disable_watchdog.sh start 99 2 3 4 5 .

    install -m 0755 ${WORKDIR}/enable_watchdog_ext_signal.sh ${D}${sysconfdir}/init.d/enable_watchdog_ext_signal.sh
    update-rc.d -r ${D} enable_watchdog_ext_signal.sh start 99 2 3 4 5 .

    localbindir="/usr/local/bin"
    #Install tunnelling  
    install -m 0755 ${WORKDIR}/remote_cpu_access.sh ${D}/usr/local/bin/remote_cpu_access.sh
    install -m 0755 ${WORKDIR}/tunnel_setup.sh ${D}${sysconfdir}/init.d/tunnel_setup.sh
    install -m 0755 ${WORKDIR}/enable_flash_cs1.sh ${D}${sysconfdir}/init.d/enable_flash_cs1.sh
    install -m 0755 ${WORKDIR}/which_flash.sh ${D}${sysconfdir}/init.d/which_flash.sh
    install -m 0755 lsb_release ${D}${localbindir}/lsb_release
    install -m 0755 bolt_meminfo ${D}${localbindir}/bolt_meminfo
    install -m 0755 bolt_cpuinfo ${D}${localbindir}/bolt_cpuinfo

    install -m 0755 bolt_SYSCPLD_ver0p6.jbc ${D}${localbindir}/bolt_SYSCPLD_ver0p6.jbc
    install -m 0755 which_flash.sh ${D}${localbindir}/which_flash.sh
    install -m 0755 flash-upg ${D}${localbindir}/flash-upg
    install -m 0755 clear-boot-sel ${D}${localbindir}/clear-boot-sel
    install -m 0755 boot0 ${D}${localbindir}/boot0

    update-rc.d -r ${D} tunnel_setup.sh start 99 2 3 4 5 .
    update-rc.d -r ${D} which_flash.sh start 99 2 3 4 5 .
}

FILES_${PN} += "${sysconfdir}"
