include ./Make.defines

PROGS =	select_TCP_recv_urgent_data

all:	${PROGS}

select_TCP_recv_urgent_data:	select_TCP_recv_urgent_data.o
		${CC} ${CFLAGS} -o $@ select_TCP_recv_urgent_data.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
