CC = m68k-elf-gcc -m68030 -O2 -I./ -I../src -c

LIBCOBJ = vgs_system.o
LIBCOBJ += vgs_cg.o
LIBCOBJ += vgs_stdlib.o
LIBCOBJ += vgs_string.o
LIBCOBJ += vgs_math.o
LIBCOBJ += vgs_ctype.o
LIBCOBJ += vgs_save.o
LIBCOBJ += vgs_key.o
LIBCOBJ += vgs_bmpfont.o

all: libc.a liblog.a
	cp -p vgs_io.h ../src

clean:
	rm -f *.a
	rm -f *.o

libc.a: ${LIBCOBJ}
	m68k-elf-ar rcs $@ ${LIBCOBJ}
 
liblog.a: log.o
	m68k-elf-ar rcs $@ $<

.SUFFIXES: .c .o

.c.o:
	${CC} $<
