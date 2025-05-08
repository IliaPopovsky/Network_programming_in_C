include ./Make.defines

PROGS =	broadcast_daytime_UDP_client

all:	${PROGS}

broadcast_daytime_UDP_client:	broadcast_daytime_UDP_client.o
		${CC} ${CFLAGS} -o $@ broadcast_daytime_UDP_client.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
