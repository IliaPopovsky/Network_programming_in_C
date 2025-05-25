include ./Make.defines

PROGS =	names_echo_UDP_server_monitoring_all_interfaice

all:	${PROGS}

names_echo_UDP_server_monitoring_all_interfaice:	names_echo_UDP_server_monitoring_all_interfaice.o
		${CC} ${CFLAGS} -o $@ names_echo_UDP_server_monitoring_all_interfaice.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
