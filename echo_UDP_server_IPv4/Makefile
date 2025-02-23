include ./Make.defines

PROGS =	echo_UDP_server_IPv4

all:	${PROGS}

echo_UDP_server_IPv4:	echo_UDP_server_IPv4.o
		${CC} ${CFLAGS} -o $@ echo_UDP_server_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
