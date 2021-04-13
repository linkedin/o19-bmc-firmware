#!/bin/bash
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

. /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

prog="$0"

gige_mod_reset_avail=0
#GIGE_MOD_RST_SYSFS=""

PWR_SYSTEM_SYSFS="${SYSCPLD_SYSFS_DIR}/pwr_cyc_all_n"
PWR_USRV_RST_SYSFS="${SYSCPLD_SYSFS_DIR}/usrv_rst_n"
PWR_TH_RST_SYSFS="${SYSCPLD_SYSFS_DIR}/th_sys_rst_n"

COME_PHY_RST_SYSFS="${SYSCPLD_SYSFS_DIR}/com-e_phy_rst_n"
OB_PHY_RST_SYSFS="${SYSCPLD_SYSFS_DIR}/ob_phy_rst_n"
FP_PHY_RST_SYSFS="${SYSCPLD_SYSFS_DIR}/fp_phy_rst_n"
OOB_BCM5387_RST_SYSFS="${SYSCPLD_SYSFS_DIR}/oob_bcm5387_rst_n"
FAN_RCKMON_RST_SYSFS="${SYSCPLD_SYSFS_DIR}/fan_rackmon_rst_n"

usage() {
    echo "Usage: $prog <command> [command options]"
    echo
    echo "Commands:"
    echo "  status  : COMe power status"
    echo
    echo "  all     : Power cycle the whole switch"
    echo
    echo "  tmhk    : Power cycle the Tomahawk switch"
    echo
    echo "  5387    : Power cycle the OOB switch (BCM5387)"
    echo
    echo "  usb     : Power cycle the USB Controller"
    echo
    echo "  fans    : Power cycle the Fan Controller"
    echo
    echo "  come    : Power cycle the COMe"
    echo
    echo "  come-phy: Power cycle the COMe dual PHY (BCM5482)"
    echo
    echo "  gige    : Power cycle the Gige PHY module"
    echo
    echo "  ob-phy  : Power cycle the On Board PHY"
    echo
    echo "  fr-phy  : Power cycle the Front Panel PHY (BCM5461s)"
    echo
}

come_status() {
    echo -n "COMe power is "
    if wedge_is_us_on; then
        echo "ON"
    else
        echo "OFF"
    fi
    return 0
}

reset_usb() {
    reset_usb.sh
    return 0
}

reset_tmhk() {
    reset_brcm.sh
    return 0
}

reset_bcm5387() {
    echo -n "Reset OOB BCM5387 ..."
    echo 1 > $OOB_BCM5387_RST_SYSFS
    sleep 1
    echo 0 > $OOB_BCM5387_RST_SYSFS
    logger "Successfully reset OOB BCM5387"
    return 0
}

reset_gige_mod() {
    if gige_mod_reset_avail; then
      echo -n "Reset GigE Module PHY ..."
      echo 0 > $GIGE_MOD_RST_SYSFS
      sleep 1
      echo 1 > $GIGE_MOD_RST_SYSFS
      logger "Successfully reset GigE Module PHY"
    else
      echo -n "GigE Module PHY Reset not supported..."
      return 1
    fi
    return 0
}

reset_dual_phy() {
    echo -n "Reset COMe (Dual) PHY ..."
    echo 0 > $COME_PHY_RST_SYSFS
    sleep 1
    echo 1 > $COME_PHY_RST_SYSFS
    logger "Successfully reset COMe (Dual) PHY"
    return 0
}

reset_ob_phy() {
    echo -n "Reset On Board PHY ..."
    echo 0 > $OB_PHY_RST_SYSFS
    sleep 1
    echo 1 > $OB_PHY_RST_SYSFS
    logger "Successfully reset On Board PHY"
    return 0
}

reset_fp_phy() {
    echo -n "Reset Front Port PHY ..."
    echo 0 > $FP_PHY_RST_SYSFS
    sleep 1
    echo 1 > $FP_PHY_RST_SYSFS
    logger "Successfully reset Front Port PHY"
    return 0
}

reset_fan_controller() {
    echo -n "Reset Fan Controller ..."
    echo 1 > $FAN_RCKMON_RST_SYSFS
    sleep 1
    echo 0 > $FAN_RCKMON_RST_SYSFS
    logger "Successfully reset Fan Controller"
    return 0
}

reset_come() {
    echo -n "Reset COMe ..."
    echo -n "  First resetting Tomahawk"
    reset_brcm.sh
    echo -n "  Tomahawk reset done."
    echo 0 > $PWR_USRV_RST_SYSFS
    sleep 1
    echo 1 > $PWR_USRV_RST_SYSFS
    logger "Successfully Reset COMe"
    return 0
}

reset_all() {
    pulse_us=100000             # 100ms
    logger "Power reset the whole system ..."
    echo -n "Power reset the whole system ..."
    sleep 1
    echo 0 > $PWR_SYSTEM_SYSFS
    # Echo 0 above should work already. However, after CPLD upgrade,
    # We need to re-generate the pulse to make this work
    usleep $pulse_us
    echo 1 > $PWR_SYSTEM_SYSFS
    usleep $pulse_us
    echo 0 > $PWR_SYSTEM_SYSFS
    usleep $pulse_us
    echo 1 > $PWR_SYSTEM_SYSFS
    echo " Done"
    return 0
}

if [ $# -lt 1 ]; then
    usage
    exit -1
fi

command="$1"
shift

case "$command" in
    status)
        come_status $@
        ;;
    all)
        reset_all $@
        ;;
    tmhk)
        reset_tmhk $@
        ;;
    5387)
        reset_bcm5387 $@
        ;;
    usb)
        reset_usb $@
        ;;
    fans)
        reset_fan_controller $@
        ;;
    come)
        reset_come $@
        ;;
    come-phy)
        reset_dual_phy $@
        ;;
    gige)
        reset_gige_mod $@
        ;;
    ob-phy)
        reset_ob_phy $@
        ;;
    fr-phy)
        reset_fp_phy $@
        ;;
    *)
        usage
        exit -1
        ;;
esac

exit $?
