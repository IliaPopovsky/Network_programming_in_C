include ./Make.defines

PROGS =	checkopts

all:	${PROGS}

checkopts:	checkopts.o
		${CC} ${CFLAGS} -o $@ checkopts.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
