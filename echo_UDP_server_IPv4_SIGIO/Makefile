include ./Make.defines

PROGS =	echo_UDP_server_IPv4_SIGIO

all:	${PROGS}

echo_UDP_server_IPv4:	echo_UDP_server_IPv4_SIGIO.o
		${CC} ${CFLAGS} -o $@ echo_UDP_server_IPv4_SIGIO.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
