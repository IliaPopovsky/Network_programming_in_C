include ./Make.defines

PROGS =	select_echo_TCP_server_IPv4

all:	${PROGS}

select_echo_TCP_server_IPv4:	select_echo_TCP_server_IPv4.o
		${CC} ${CFLAGS} -o $@ select_echo_TCP_server_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
