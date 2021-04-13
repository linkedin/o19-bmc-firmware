# Copyright 2015-present Facebook. All Rights Reserved.
lib: libpal.so

libpal.so: pal.c
	$(CC) $(CFLAGS) -fPIC -c -pthread -o pal.o pal.c
	$(CC) -lbic -lyellowsub_common -lyellowsub_fruid -lyellowsub_sensor -lkv -ledb -shared -o libpal.so pal.o -lc -lrt -Wl,--whole-archive -lobmc-pal -lpam -Wl,--no-whole-archive

.PHONY: clean

clean:
	rm -rf *.o libpal.so
