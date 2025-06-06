include ./Make.defines

PROGS =	TCP_send_urgent_data

all:	${PROGS}

TCP_send_urgent_data:	TCP_send_urgent_data.o
		${CC} ${CFLAGS} -o $@ TCP_send_urgent_data.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
