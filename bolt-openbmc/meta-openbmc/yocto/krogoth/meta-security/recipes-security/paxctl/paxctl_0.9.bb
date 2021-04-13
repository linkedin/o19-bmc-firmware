DESCRIPTION = "paxctl  is  a tool that allows PaX flags to be modified on a \
               per-binary basis. PaX is part of common  security-enhancing  \
               kernel  patches  and secure distributions, such as \
               GrSecurity or Adamantix and Hardened Gen-too, respectively."
HOMEPAGE = "https://pax.grsecurity.net/"	       
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://paxctl.c;beginline=1;endline=5;md5=0ddd065c61020dda79729e6bedaed2c7 \
                    file://paxctl-elf.c;beginline=1;endline=5;md5=99f453ce7f6d1687ee808982e2924813 \
		   "

SRC_URI = "http://pax.grsecurity.net/${BP}.tar.gz"

SRC_URI[md5sum] = "9bea59b1987dc4e16c2d22d745374e64"
SRC_URI[sha256sum] = "a330ddd812688169802a3ba29e5e3b19956376b8f6f73b8d7e9586eb04423c2e"

EXTRA_OEMAKE = "CC='${CC}' DESTDIR='${D}'"

do_install() {
	oe_runmake install
}
