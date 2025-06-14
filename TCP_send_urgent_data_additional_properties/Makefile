include ./Make.defines

PROGS =	TCP_send_urgent_data_additional_properties

all:	${PROGS}

TCP_send_urgent_data_additional_properties:	TCP_send_urgent_data_additional_properties.o
		${CC} ${CFLAGS} -o $@ TCP_send_urgent_data_additional_properties.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
