include ./Make.defines

PROGS =	daemon_names_daytime_TCP_server

all:	${PROGS}

daemon_names_daytime_TCP_server:	daemon_names_daytime_TCP_server.o
		${CC} ${CFLAGS} -o $@ daemon_names_daytime_TCP_server.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
