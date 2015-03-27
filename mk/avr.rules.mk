.SUFFIXES: .c .cpp .o .i .ii .s

.c.o:
	${CC} ${CFLAGS} -c -o ${.TARGET} ${.IMPSRC}

.cpp.o:
	${CXX} ${CXXFLAGS} -c -o ${.TARGET} ${.IMPSRC}

.c.i:
	${CC} ${CFLAGS} -E ${.IMPSRC} >${.TARGET}

.cpp.ii:
	${CXX} ${CXXFLAGS} -E ${.IMPSRC} >${.TARGET}

.c.s:
	${CC} ${CFLAGS} -S ${.IMPSRC} >${.TARGET}

.cpp.s:
	${CXX} ${CXXFLAGS} -S ${.IMPSRC} >${.TARGET}
