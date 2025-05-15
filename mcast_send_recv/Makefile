include ./Make.defines

PROGS =	mcast_send_recv

all:	${PROGS}

mcast_send_recv:	mcast_send_recv.o
		${CC} ${CFLAGS} -o $@ mcast_send_recv.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
