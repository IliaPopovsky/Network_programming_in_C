include ./Make.defines

PROGS =	names_paral_echo_TCP_server_threads

all:	${PROGS}

names_paral_echo_TCP_server_threads:	names_paral_echo_TCP_server_threads.o
		${CC} ${CFLAGS} -o $@ names_paral_echo_TCP_server_threads.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
