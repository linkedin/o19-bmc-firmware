LINUX_VERSION_EXTENSION = "-wedge100"

COMPATIBLE_MACHINE = "wedge100"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://defconfig \
            file://0001-Adding-dual-flash-boot-support.patch \
           "
KERNEL_MODULE_AUTOLOAD += " \
  tpm \
  tpm_i2c_infineon \
"
