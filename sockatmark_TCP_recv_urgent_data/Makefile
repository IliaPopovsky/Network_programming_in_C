include ./Make.defines

PROGS =	sockatmark_TCP_recv_urgent_data

all:	${PROGS}

sockatmark_TCP_recv_urgent_data:	sockatmark_TCP_recv_urgent_data.o
		${CC} ${CFLAGS} -o $@ sockatmark_TCP_recv_urgent_data.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
