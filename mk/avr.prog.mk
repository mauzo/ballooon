.if !target(__<avr.prog.mk>__)
__<avr.prog.mk>__:

OBJS?=		${SRCS:R:S/$/.o/g}
CLEANFILES+=	${PROG} ${OBJS}
OBJDIRS+=	${SRCS:H:u}

.if !empty(LD) && ${LD} == ld
.  undef LD
.endif

.if !empty(SRCS:M*.cpp)
LD?=	${CXX}
.else
LD?=	${CC}
.endif

.ifdef AMD
all:	${PROG}
.else
CLEANFILES+=	${PROG}.hex ${PROG}.eep

all:	${PROG}.hex ${PROG}.eep
.endif

.include "avr.obj.mk"
.include "avr.dep.mk"
.include "avr.rules.mk"

${PROG}: ${OBJS}
	${LD} ${LDFLAGS} -o ${.TARGET} ${.ALLSRC} ${LIBS}

${PROG}.hex: ${PROG}
	${OBJCOPY} -O ihex -R .eeprom ${PROG} ${.TARGET}

${PROG}.eep: ${PROG}
	${OBJCOPY} -O ihex -j .eeprom \
		--set-section-flags=.eeprom=alloc,load \
		--no-change-warnings \
		--change-section-lma .eeprom=0 \
		${PROG} ${.TARGET}

.endif
