
SRCBRANCH = "openbmc/helium/4.1"
SRCREV = "7c5d8d4679e38f647ad872989d044855698e361a"

SRC_URI = "git://github.com/theopolis/linux.git;branch=${SRCBRANCH};protocol=https \
          "

LINUX_VERSION ?= "4.1.51"
LINUX_VERSION_EXTENSION ?= "-aspeed"

PR = "r1"
PV = "${LINUX_VERSION}+${SRCREV}"

S = "${WORKDIR}/git"

include linux-aspeed.inc
