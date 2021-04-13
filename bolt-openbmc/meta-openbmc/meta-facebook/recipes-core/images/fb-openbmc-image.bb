# Copyright 2018-present Facebook. All Rights Reserved.

# Base this image on core-image-minimal
require recipes-core/images/core-image-minimal.bb

ROOTFS_POSTPROCESS_COMMAND_append += " openbmc_rootfs_fixup; "

OPENBMC_HOSTNAME ?= "bmc"

openbmc_rootfs_fixup() {
    # hostname
    if [ "${OPENBMC_HOSTNAME}" != "" ]; then
        echo ${OPENBMC_HOSTNAME} > ${IMAGE_ROOTFS}/etc/hostname
    else
        echo ${MACHINE} > ${IMAGE_ROOTFS}/etc/hostname
    fi

    # version
    echo "Bolt OpenBMC Release 2.2.0" > ${IMAGE_ROOTFS}/etc/issue
    echo >> ${IMAGE_ROOTFS}/etc/issue
    echo "Bolt OpenBMC Release 2.2.0 %h" > ${IMAGE_ROOTFS}/etc/issue.net
    echo >> ${IMAGE_ROOTFS}/etc/issue.net
}
