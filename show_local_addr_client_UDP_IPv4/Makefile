include ./Make.defines

PROGS =	show_local_addr_client_UDP_IPv4

all:	${PROGS}

show_local_addr_client_UDP_IPv4:	show_local_addr_client_UDP_IPv4.o
		${CC} ${CFLAGS} -o $@ show_local_addr_client_UDP_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
