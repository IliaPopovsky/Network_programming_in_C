include ./Make.defines

PROGS =	daytimetcpclient_MOD

all:	${PROGS}

daytimetcpclient_MOD:	daytimetcpclient_MOD.o
		${CC} ${CFLAGS} -o $@ daytimetcpclient_MOD.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
