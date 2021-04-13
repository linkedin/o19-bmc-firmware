lib: libethcfg.so

libethcfg.so: ethcfg.c
	$(CC) $(CFLAGS) -fPIC -c -o ethcfg.o ethcfg.c -ljansson
	$(CC) -lm -shared -o libethcfg.so ethcfg.o -lc -ljansson

.PHONY: clean

clean:
	rm -rf *.o libethcfg.so
