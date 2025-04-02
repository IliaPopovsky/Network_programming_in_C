include ./Make.defines

PROGS =	names_daytime_TCP_client

all:	${PROGS}

names_daytime_TCP_client:	names_daytime_TCP_client.o
		${CC} ${CFLAGS} -o $@ names_daytime_TCP_client.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
