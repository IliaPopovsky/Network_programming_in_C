include ./Make.defines

PROGS =	poll_echo_TCP_server_IPv4

all:	${PROGS}

poll_echo_TCP_server_IPv4:	poll_echo_TCP_server_IPv4.o
		${CC} ${CFLAGS} -o $@ poll_echo_TCP_server_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
