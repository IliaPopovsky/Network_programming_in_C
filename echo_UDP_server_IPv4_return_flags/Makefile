include ./Make.defines

PROGS =	echo_UDP_server_IPv4_return_flags

all:	${PROGS}

echo_UDP_server_IPv4_return_flags:	echo_UDP_server_IPv4_return_flags.o
		${CC} ${CFLAGS} -o $@ echo_UDP_server_IPv4_return_flags.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
