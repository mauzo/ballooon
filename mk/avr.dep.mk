.PHONY: depend ${DEPENDFILE} cleandepend

DEPENDFILE=	${.OBJDIR}/.depend

depend: ${DEPENDFILE}

${DEPENDFILE}: ${SRCS}
	echo >${.TARGET}
.if !empty(SRCS:M*.c)
	${CC} -MM ${CFLAGS} ${.ALLSRC:M*.c} >>${.TARGET}
.endif
.if !empty(SRCS:M*.cpp)
	${CXX} -MM ${CXXFLAGS} ${.ALLSRC:M*.cpp} >>${.TARGET}
.endif
.if !empty(SRCS:M*.ino)
	${CXX} -MM ${CXXFLAGS} -x c++ ${.ALLSRC:M*.ino} >>${.TARGET}
.endif

cleandepend:
	rm -f ${DEPENDFILE}
