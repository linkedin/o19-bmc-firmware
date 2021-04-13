## LinkedIn OpenBMC
OpenBMC is an open software framework to build a complete Linux image for a Board Management Controller (BMC). The BMC software for Lightning powershelf and Bolt switch are based on [facebook OpenBMC framework](https://github.com/facebook/openbmc).

### Instructions to build and load BMC firmware
##### Getting the source code:
1) mkdir bmc
2) cd bmc
3) git clone -b rocko [https://git.yoctoproject.org/git/poky](https://git.yoctoproject.org/git/poky)
4) cd poky
5) git clone -b rocko  [https://github.com/openembedded/meta-openembedded.git](https://github.com/openembedded/meta-openembedded.git)
6) git clone -b rocko [https://git.yoctoproject.org/git/meta-security](https://git.yoctoproject.org/git/meta-security)
7) git clone https://github.com/linkedin/o19-bmc-firmware.git .

##### Build Lightning:
export TEMPLATECONF=meta-openbmc/meta-linkedin/meta-lighting/conf
source oe-init-build-env
bitbake lighting-image

Image path: poky/build/tmp/deploy/images/lighting/flash-lighting
##### Build Bolt switch:
cd bolt-openbmc/meta-openbmc
source ./openbmc-init-build-env meta-flex/meta-bolt
bitbake wedge100-image

Image path: meta-openbmc/meta-flex/meta-bolt/tmp/deploy/images/wedge100/flash-wedge100
