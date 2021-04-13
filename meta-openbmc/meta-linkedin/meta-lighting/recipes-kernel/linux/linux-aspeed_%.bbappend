LINUX_VERSION_EXTENSION = "-lighting"

COMPATIBLE_MACHINE = "lighting"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://defconfig \
            file://li_kernel.patch \
           "
KERNEL_MODULE_AUTOLOAD += " \
  tpm \
  tpm_i2c_infineon \
"

KERNEL_MODULE_PROBECONF += "                    \
 i2c-mux-pca954x                                \
 i2c-mux-pca9541                                \
 lm25066                                        \
"

module_conf_i2c-mux-pca954x = "options i2c-mux-pca954x ignore_probe=1"
module_conf_i2c-mux-pca9541 = "options i2c-mux-pca9541 ignore_probe=1"
module_conf_lm25066 = "options lm25066 ignore_probe=1"
