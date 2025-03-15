include ./Make.defines

PROGS =	select_paral_echo_TCP_UDP_server_IPv4

all:	${PROGS}

select_paral_echo_TCP_UDP_server_IPv4:	select_paral_echo_TCP_UDP_server_IPv4.o
		${CC} ${CFLAGS} -o $@ select_paral_echo_TCP_UDP_server_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
