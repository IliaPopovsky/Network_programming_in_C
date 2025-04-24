include ./Make.defines

PROGS =	paral_echo_UDS_STREAM_server

all:	${PROGS}

paral_echo_UDS_STREAM_server:	paral_echo_UDS_STREAM_server.o
		${CC} ${CFLAGS} -o $@ paral_echo_UDS_STREAM_server.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
