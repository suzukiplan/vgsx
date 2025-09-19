CC = m68k-elf-gcc -m68030 -O2 -I./ -c

LIBCOBJ = vgs_system.o
LIBCOBJ += vgs_cg.o
LIBCOBJ += vgs_bgm.o
LIBCOBJ += vgs_sfx.o
LIBCOBJ += vgs_stdlib.o
LIBCOBJ += vgs_string.o

all: libc.a liblog.a

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
