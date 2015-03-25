.SUFFIXES: .c .cpp .o

.c.o:
	${CC} ${CFLAGS} -c -o ${.TARGET} ${.IMPSRC}

.cpp.o:
	${CXX} ${CXXFLAGS} -c -o ${.TARGET} ${.IMPSRC}
