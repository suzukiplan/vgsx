all: libc.a liblog.a

libc.a: libc.c stdint.h vgs.h
	m68k-elf-gcc -m68030 -O2 -I./ -c libc.c
	m68k-elf-ar rcs libc.a libc.o
 
liblog.a: log.c vgs.h
	m68k-elf-gcc -m68030 -O2 -I./ -c log.c
	m68k-elf-ar rcs liblog.a log.o
