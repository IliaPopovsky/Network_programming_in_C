include ./Make.defines

PROGS =	paral_echo_TCP_client_IPv4_threads

all:	${PROGS}

paral_echo_TCP_client_IPv4_threads:	paral_echo_TCP_client_IPv4_threads.o
		${CC} ${CFLAGS} -o $@ paral_echo_TCP_client_IPv4_threads.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
