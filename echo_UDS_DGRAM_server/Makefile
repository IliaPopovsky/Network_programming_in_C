include ./Make.defines

PROGS =	echo_UDS_DGRAM_server

all:	${PROGS}

echo_UDS_DGRAM_server:	echo_UDS_DGRAM_server.o
		${CC} ${CFLAGS} -o $@ echo_UDS_DGRAM_server.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
