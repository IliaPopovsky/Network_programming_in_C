include ./Make.defines

PROGS =	names_daytime_UDP_client

all:	${PROGS}

names_daytime_UDP_client:	names_daytime_UDP_client.o
		${CC} ${CFLAGS} -o $@ names_daytime_UDP_client.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
