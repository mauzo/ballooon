.if !target(__<avr.prog.mk>__)
__<avr.prog.mk>__:

OBJS?=		${SRCS:R:S/$/.o/g}
CLEANFILES+=	${PROG} ${OBJS}
OBJDIRS+=	${SRCS:H:u}

AVRDUDE?=	avrdude
CU?=		cu

.if !empty(LD) && ${LD} == ld
.  undef LD
.endif

.if !empty(SRCS:M*.cpp)
LD?=	${CXX}
.else
LD?=	${CC}
.endif

.PHONY:	upload tty

all:	${PROG}

upload:	${PROG}
	${AVRDUDE} -p${AVRDUDE_MCU} -c${AVRDUDE_PROG} -P${AVRDUDE_TTY} -U ${PROG}

tty:	upload
	${CU} -l${AVRDUDE_TTY} -s${TTY_SPEED}

.include "avr.obj.mk"
.include "avr.dep.mk"
.include "avr.rules.mk"

${PROG}: ${OBJS}
	${LD} ${LDFLAGS} -o ${.TARGET} ${.ALLSRC} ${LIBS}

.endif
