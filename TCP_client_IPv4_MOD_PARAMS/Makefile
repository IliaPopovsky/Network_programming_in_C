include ./Make.defines

PROGS =	daytimetcpclient_MOD_PARAMS

all:	${PROGS}

daytimetcpclient_MOD_PARAMS:	daytimetcpclient_MOD_PARAMS.o
		${CC} ${CFLAGS} -o $@ daytimetcpclient_MOD_PARAMS.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
