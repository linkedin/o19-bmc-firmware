LINUX_VERSION_EXTENSION = "-yellowsub"

COMPATIBLE_MACHINE = "yellowsub"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://defconfig \
            file://yellowsub.patch \
            file://vlan_ncsi.patch \
           "

KERNEL_MODULE_AUTOLOAD += " \
"

KERNEL_MODULE_PROBECONF += "                    \
 i2c-mux-pca954x                                \
"

module_conf_i2c-mux-pca954x = "options i2c-mux-pca954x ignore_probe=1"

