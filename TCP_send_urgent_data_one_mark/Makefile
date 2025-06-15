include ./Make.defines

PROGS =	TCP_send_urgent_data_one_mark

all:	${PROGS}

TCP_send_urgent_data_one_mark:	TCP_send_urgent_data_one_mark.o
		${CC} ${CFLAGS} -o $@ TCP_send_urgent_data_one_mark.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
