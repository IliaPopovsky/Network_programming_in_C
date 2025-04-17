include ./Make.defines

PROGS =	paral_echo_TCP_client_IPv4

all:	${PROGS}

paral_echo_TCP_client_IPv4:	paral_echo_TCP_client_IPv4.o
		${CC} ${CFLAGS} -o $@ paral_echo_TCP_client_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
