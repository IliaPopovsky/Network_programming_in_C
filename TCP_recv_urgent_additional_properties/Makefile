include ./Make.defines

PROGS =	TCP_recv_urgent_additional_properties

all:	${PROGS}

TCP_recv_urgent_additional_properties:	TCP_recv_urgent_additional_properties.o
		${CC} ${CFLAGS} -o $@ TCP_recv_urgent_additional_properties.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
