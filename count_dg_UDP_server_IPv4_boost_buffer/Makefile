include ./Make.defines

PROGS =	count_dg_UDP_server_IPv4_boost_buffer

all:	${PROGS}

count_dg_UDP_server_IPv4_boost_buffer:	count_dg_UDP_server_IPv4_boost_buffer.o
		${CC} ${CFLAGS} -o $@ count_dg_UDP_server_IPv4_boost_buffer.o ${LIBS}



clean:
		rm -f ${PROGS} ${CLEANFILES}
