include ./Make.defines

PROGS =	echo_UDP_client_IPv4_connect

all:	${PROGS}

echo_UDP_client_IPv4_connect:	echo_UDP_client_IPv4_connect.o
		${CC} ${CFLAGS} -o $@ echo_UDP_client_IPv4_connect.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
