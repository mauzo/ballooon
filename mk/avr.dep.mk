.PHONY: depend cleandepend

DEPENDFILE=	${.OBJDIR}/.depend

depend: ${DEPENDFILE}

${DEPENDFILE}: ${SRCS}
	echo >${.TARGET}
.if !empty(SRCS:M*.c)
	${CC} -MM ${CFLAGS} ${SRCS:M*.c} >>${.TARGET}
.endif
.if !empty(SRCS:M*.cpp)
	${CXX} -MM ${CXXFLAGS} ${SRCS:M*.cpp} >>${.TARGET}
.endif
.if !empty(SRCS:M*.ino)
	${CXX} -MM ${CXXFLAGS} -x c++ ${SRCS:M*.ino} >>${.TARGET}
.endif

cleandepend:
	rm -f ${DEPENDFILE}
