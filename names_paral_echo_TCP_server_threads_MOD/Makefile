include ./Make.defines

PROGS =	names_paral_echo_TCP_server_threads_MOD

all:	${PROGS}

names_paral_echo_TCP_server_threads_MOD:	names_paral_echo_TCP_server_threads_MOD.o
		${CC} ${CFLAGS} -o $@ names_paral_echo_TCP_server_threads_MOD.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
