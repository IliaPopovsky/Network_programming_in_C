include ./Make.defines

PROGS =	more_reliable_echo_UDP_client_IPv4

all:	${PROGS}

more_reliable_echo_UDP_client_IPv4:	more_reliable_echo_UDP_client_IPv4.o
		${CC} ${CFLAGS} -o $@ more_reliable_echo_UDP_client_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
