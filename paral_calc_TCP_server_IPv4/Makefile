include ./Make.defines

PROGS =	paral_calc_TCP_server_IPv4

all:	${PROGS}

paral_calc_TCP_server_IPv4:	paral_calc_TCP_server_IPv4.o
		${CC} ${CFLAGS} -o $@ paral_calc_TCP_server_IPv4.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
