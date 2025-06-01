include ./Make.defines

PROGS =	TCP_recv_urgent_data

all:	${PROGS}

TCP_recv_urgent_data:	TCP_recv_urgent_data.o
		${CC} ${CFLAGS} -o $@ TCP_recv_urgent_data.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
