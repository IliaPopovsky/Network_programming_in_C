include ./Make.defines

PROGS =	echo_TCP_client_IPv4

all:	${PROGS}

echo_TCP_client_IPv4:	echo_TCP_client_IPv4.o
		${CC} ${CFLAGS} -o $@ echo_TCP_client_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
