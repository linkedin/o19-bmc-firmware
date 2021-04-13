lib: libpowershelf.so

libpowershelf.so: powershelf_efuse.c powershelf_psu.c powershelf_fan.c powershelf_common.c
	$(CC) $(CFLAGS) -fPIC -c -o powershelf_efuse.o powershelf_efuse.c -lgpio
	$(CC) $(CFLAGS) -fPIC -c -o powershelf_psu.o powershelf_psu.c -lgpio
	$(CC) $(CFLAGS) -fPIC -c -o powershelf_fan.o powershelf_fan.c -lgpio
	$(CC) $(CFLAGS) -fPIC -c -o powershelf_common.o powershelf_common.c -lgpio
	$(CC) -lm -shared -o libpowershelf.so powershelf_efuse.o powershelf_psu.o powershelf_fan.o powershelf_common.o -lc -lgpio

.PHONY: clean

clean:
	rm -rf *.o libpowershelf.so
