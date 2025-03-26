include ./Make.defines

PROGS =	echo_UDS_client

all:	${PROGS}

echo_UDS_client:	echo_UDS_client.o
		${CC} ${CFLAGS} -o $@ echo_UDS_client.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
