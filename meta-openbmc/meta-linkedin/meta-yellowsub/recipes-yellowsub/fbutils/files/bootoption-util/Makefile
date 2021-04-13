# Copyright 2018-present LinkedIn. All Rights Reserved.
all: bootoption-util

bootoption-util: bootoption-util.c
	$(CC) -lpal -std=c99 -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o bootoption-util
