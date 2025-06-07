include ./Make.defines

PROGS =	TCP_send_urgent_data_MOD

all:	${PROGS}

TCP_send_urgent_data_MOD:	TCP_send_urgent_data_MOD.o
		${CC} ${CFLAGS} -o $@ TCP_send_urgent_data_MOD.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
