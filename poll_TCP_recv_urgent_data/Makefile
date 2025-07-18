include ./Make.defines

PROGS =	poll_TCP_recv_urgent_data

all:	${PROGS}

poll_TCP_recv_urgent_data:	poll_TCP_recv_urgent_data.o
		${CC} ${CFLAGS} -o $@ poll_TCP_recv_urgent_data.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
