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

utils: onie_eep_dbg onie-eeprom-prog onie-eeprom-show onie-eeprom-upg id-eeprom-prog id-eeprom-upg id-eeprom-show
#all: weutil wemanuf id-eeprom-upg onie_eep_dbg onie-eeprom-prog onie-eeprom-show onie-eeprom-upg id-eeprom-prog id-eeprom-show

#weutil: weutil.o
#	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

#wemanuf: wemanuf.o
#	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

id-eeprom-upg: id-eeprom-upg.o
	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

onie_eep_dbg: onie_eep_dbg.o
	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

onie-eeprom-prog: onie-eeprom-prog.o
	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

onie-eeprom-show: onie-eeprom-show.o
	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

onie-eeprom-upg: onie-eeprom-upg.o
	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

id-eeprom-prog: id-eeprom-prog.o
	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

id-eeprom-show: id-eeprom-show.o
	$(CC) $(LDFLAGS) -o $@ $^ -lflex-eeprom

.PHONY: clean

clean:
	rm -rf *.o id-eeprom-upg onie_eep_dbg onie-eeprom-prog onie-eeprom-show onie-eeprom-upg id-eeprom-prog id-eeprom-show
