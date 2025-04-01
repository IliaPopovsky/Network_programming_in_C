include ./Make.defines

PROGS =	names_daytime_TCP_server

all:	${PROGS}

names_daytime_TCP_server:	names_daytime_TCP_server.o
		${CC} ${CFLAGS} -o $@ names_daytime_TCP_server.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
