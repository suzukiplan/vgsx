CC = m68k-elf-gcc -m68030 -O2 -I./ -c

LIBCOBJ = system.o
LIBCOBJ += cg.o
LIBCOBJ += bgm.o
LIBCOBJ += sfx.o
LIBCOBJ += stdlib.o
LIBCOBJ += string.o

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
