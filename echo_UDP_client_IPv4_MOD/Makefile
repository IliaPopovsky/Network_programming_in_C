include ./Make.defines

PROGS =	echo_UDP_client_IPv4_MOD

all:	${PROGS}

echo_UDP_client_IPv4_MOD:	echo_UDP_client_IPv4_MOD.o
		${CC} ${CFLAGS} -o $@ echo_UDP_client_IPv4_MOD.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
