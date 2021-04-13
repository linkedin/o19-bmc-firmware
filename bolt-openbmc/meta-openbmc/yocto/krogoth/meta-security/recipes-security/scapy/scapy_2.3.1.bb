SUMMARY = "Network scanning and manipulation tool"
DESCRIPTION = "Scapy is a powerful interactive packet manipulation program. It is able to forge or decode packets of a wide number of protocols, send them on the wire, capture them, match requests and replies, and much more. It can easily handle most classical tasks like scanning, tracerouting, probing, unit tests, attacks or network discovery (it can replace hping, 85% of nmap, arpspoof, arp-sk, arping, tcpdump, tethereal, p0f, etc.). It also performs very well at a lot of other specific tasks that most other tools can't handle, like sending invalid frames, injecting your own 802.11 frames, combining technics (VLAN hopping+ARP cache poisoning, VOIP decoding on WEP encrypted channel, ...), etc."
SECTION = "security"
LICENSE = "GPLv2"

LIC_FILES_CHKSUM = "file://bin/scapy;beginline=9;endline=13;md5=1d5249872cc54cd4ca3d3879262d0c69"

SRC_URI = "https://bitbucket.org/secdev/${PN}/downloads/${BP}.zip"

SRC_URI[md5sum] = "a30d828e59801d1d092219b349f1da9e"
SRC_URI[sha256sum] = "8972c02e39a826a10c02c2bdd5025f7251dce9589c57befd9bb55c65f02e4934"

inherit setuptools

RDEPENDS_${PN} = "tcpdump python-subprocess python-compression python-netclient  \
                  python-netserver python-pydoc python-pkgutil python-shell \
                  python-threading python-numbers python-pycrypto"
