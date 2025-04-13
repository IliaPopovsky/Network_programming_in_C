include ./Make.defines

PROGS =	names_daytime_UDP_server

all:	${PROGS}

names_daytime_UDP_server:	names_daytime_UDP_server.o
		${CC} ${CFLAGS} -o $@ names_daytime_UDP_server.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
