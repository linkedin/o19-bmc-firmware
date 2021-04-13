FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-4.1:"

# Tomoyo kernel support
SRC_URI += "\
	${@base_contains('DISTRO_FEATURES', 'tomoyo', ' file://ccs-tools-yocto.4.1.patch', '', d)} \
	${@base_contains('DISTRO_FEATURES', 'tomoyo', ' file://ccs-tools-yocto_security.patch', '', d)} \
	${@base_contains('DISTRO_FEATURES', 'tomoyo', ' file://tomoyo.cfg', '', d)} \
	${@base_contains('DISTRO_FEATURES', 'tomoyo', ' file://tomoyo.scc', '', d)} \
	"
