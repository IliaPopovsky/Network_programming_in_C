include ./Make.defines

PROGS =	names_daytime_UDP_client_connect

all:	${PROGS}

names_daytime_UDP_client_connect:	names_daytime_UDP_client_connect.o
		${CC} ${CFLAGS} -o $@ names_daytime_UDP_client_connect.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
