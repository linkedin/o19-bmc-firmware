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

import subprocess

# Endpoint for showing BMC software version
def parse_data(output):
  data = {}
  for line in output.splitlines():
    parts = line.split(':', 1)
    if len(parts) != 2:
      continue
    key = parts[0].strip()
    val = parts[1].strip()
    data[key] = val

  return data

def get_vers():
    cmd = [ "/usr/local/bin/lsb_release" ]
    vers = subprocess.check_output(cmd)
    vers = vers.decode()
    return parse_data(vers)

if(__name__=="__main__"):
    print('{}'.format(get_vers()))
