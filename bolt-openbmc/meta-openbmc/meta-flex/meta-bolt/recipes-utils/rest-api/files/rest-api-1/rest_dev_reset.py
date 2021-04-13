#!/usr/bin/env python3
#
# Copyright 2014-present Facebook. All Rights Reserved.
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

import os
from subprocess import *

def get_resets():
   (ret, _) = Popen('/usr/local/bin/bolt_reset.sh status', \
                    shell=True, stdout=PIPE).communicate()
   ret = ret.decode()
   status = ret.rsplit()[-1]

   result = {
                "Information": { "COMe power": status },
                "Resets": ["bolt", "bcm-tmhk", "usb", "come", \
                "come-phy", "gige", "ob-phy", "fr-phy"],
            }

   return result


# Endpoint for resetting devices
def dev_reset(data):
    if data["device"] == "bolt":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh all', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "bcm-tmhk":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh tmhk', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "oob-eth":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh 5387', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "usb":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh usb', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "fans":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh fans', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "come":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh come', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "come-phy":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh come-phy', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "gige":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh gige', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "ob-phy":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh ob-phy', \
                         shell=True, stdout=PIPE).communicate()
    elif data["device"] == "fr-phy":
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh fr-phy', \
                         shell=True, stdout=PIPE).communicate()
    else:
        (ret, _) = Popen('/usr/local/bin/bolt_reset.sh status', \
                         shell=True, stdout=PIPE).communicate()
        return { "result": "FAILURE"}
    
    return { "result": "Success"}


if(__name__=="__main__"):
    print('{}'.format(get_resets()))
