include ./Make.defines

PROGS =	paral_echo_TCP_server_IPv4_MOD

all:	${PROGS}

paral_echo_TCP_server_IPv4_MOD:	paral_echo_TCP_server_IPv4_MOD.o
		${CC} ${CFLAGS} -o $@ paral_echo_TCP_server_IPv4_MOD.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
