include ./Make.defines

PROGS =	TCP_client_IPv4_connect_nonblock

all:	${PROGS}

TCP_client_IPv4_connect_nonblock:	TCP_client_IPv4_connect_nonblock.o
		${CC} ${CFLAGS} -o $@ TCP_client_IPv4_connect_nonblock.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
