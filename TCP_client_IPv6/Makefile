include ./Make.defines

PROGS =	daytimetcpclientIPv6

all:	${PROGS}

daytimetcpclientIPv6:	daytimetcpclientIPv6.o
		${CC} ${CFLAGS} -o $@ daytimetcpclientIPv6.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
