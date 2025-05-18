include ./Make.defines

PROGS =	multicast_daytime_UDP_client

all:	${PROGS}

multicast_daytime_UDP_client:	multicast_daytime_UDP_client.o
		${CC} ${CFLAGS} -o $@ multicast_daytime_UDP_client.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
