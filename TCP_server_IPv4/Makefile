include ./Make.defines

PROGS =	daytimetcpserver

all:	${PROGS}

daytimetcpserver:	daytimetcpserver.o
		${CC} ${CFLAGS} -o $@ daytimetcpserver.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
