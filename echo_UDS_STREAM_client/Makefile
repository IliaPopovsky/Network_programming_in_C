include ./Make.defines

PROGS =	echo_UDS_STREAM_client

all:	${PROGS}

echo_UDS_STREAM_client:	echo_UDS_STREAM_client.o
		${CC} ${CFLAGS} -o $@ echo_UDS_STREAM_client.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
