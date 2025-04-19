include ./Make.defines

PROGS =	echo_UDS_DGRAM_client

all:	${PROGS}

echo_UDS_DGRAM_client:	echo_UDS_DGRAM_client.o
		${CC} ${CFLAGS} -o $@ echo_UDS_DGRAM_client.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
