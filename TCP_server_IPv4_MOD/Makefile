include ./Make.defines

PROGS =	daytimetcpserver_MOD

all:	${PROGS}

daytimetcpserver_MOD:	daytimetcpserver_MOD.o
		${CC} ${CFLAGS} -o $@ daytimetcpserver_MOD.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
