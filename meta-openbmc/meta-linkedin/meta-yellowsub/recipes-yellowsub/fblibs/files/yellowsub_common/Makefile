# Copyright 2015-present Facebook. All Rights Reserved.
lib: libyellowsub_common.so

libyellowsub_common.so: yellowsub_common.c
	$(CC) $(CFLAGS) -fPIC -pthread -c -o yellowsub_common.o yellowsub_common.c
	$(CC) -lpthread -shared -o libyellowsub_common.so yellowsub_common.o -lc

.PHONY: clean

clean:
	rm -rf *.o libyellowsub_common.so
