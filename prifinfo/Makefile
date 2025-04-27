include ./Make.defines

PROGS =	prifinfo

all:	${PROGS}

prifinfo:	prifinfo.o
		${CC} ${CFLAGS} -o $@ prifinfo.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
