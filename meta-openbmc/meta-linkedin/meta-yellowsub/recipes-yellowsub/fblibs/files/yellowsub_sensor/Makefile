# Copyright 2015-present Facebook. All Rights Reserved.
lib: libyellowsub_sensor.so

libyellowsub_sensor.so: yellowsub_sensor.c
	$(CC) $(CFLAGS) -fPIC -c -o yellowsub_sensor.o yellowsub_sensor.c
	$(CC) -lm -lbic -lipmi -lipmb -lyellowsub_common -shared -o libyellowsub_sensor.so yellowsub_sensor.o -lc

.PHONY: clean

clean:
	rm -rf *.o libyellowsub_sensor.so
