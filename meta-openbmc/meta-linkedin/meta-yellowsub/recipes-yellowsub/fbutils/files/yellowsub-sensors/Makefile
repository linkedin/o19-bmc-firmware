# Copyright 2015-present Facebook. All Rights Reserved.
all: yellowsub-sensors

yellowsub-sensors: yellowsub-sensors.c
	$(CC) -lyellowsub_sensor -std=c99 -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o yellowsub-sensors
