include ./Make.defines

PROGS =	count_dg_UDP_server_IPv4

all:	${PROGS}

count_dg_UDP_server_IPv4:	count_dg_UDP_server_IPv4.o
		${CC} ${CFLAGS} -o $@ count_dg_UDP_server_IPv4.o ${LIBS}



clean:
		rm -f ${PROGS} ${CLEANFILES}
