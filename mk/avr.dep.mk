.PHONY: depend cleandepend

DEPENDFILE=	${.OBJDIR}/.depend

depend: ${DEPENDFILE}

${DEPENDFILE}: ${SRCS}
	${CC} -MM ${CFLAGS} ${.ALLSRC:M*.c} >${.TARGET}
	${CXX} -MM ${CXXFLAGS} ${.ALLSRC:M*.cpp} >>${.TARGET}
	${CXX} -MM ${CXXFLAGS} -x c++ ${.ALLSRC:M*.ino} >>${.TARGET}

cleandepend:
	rm -f ${DEPENDFILE}
