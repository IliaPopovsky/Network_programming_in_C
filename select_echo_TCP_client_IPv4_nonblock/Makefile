include ./Make.defines

PROGS =	select_echo_TCP_client_IPv4_nonblock

all:	${PROGS}

select_echo_TCP_client_IPv4_nonblock:	select_echo_TCP_client_IPv4_nonblock.o
		${CC} ${CFLAGS} -o $@ select_echo_TCP_client_IPv4_nonblock.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
