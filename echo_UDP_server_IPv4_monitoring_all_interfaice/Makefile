include ./Make.defines

PROGS =	echo_UDP_server_IPv4_monitoring_all_interfaice

all:	${PROGS}

echo_UDP_server_IPv4_monitoring_all_interfaice:	echo_UDP_server_IPv4_monitoring_all_interfaice.o
		${CC} ${CFLAGS} -o $@ echo_UDP_server_IPv4_monitoring_all_interfaice.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
