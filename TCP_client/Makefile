include ./Make.defines

PROGS =	daytimetcpclient

all:	${PROGS}

daytimetcpclient:	daytimetcpclient.o
		${CC} ${CFLAGS} -o $@ daytimetcpclient.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
