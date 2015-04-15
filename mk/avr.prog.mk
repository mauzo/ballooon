.if !target(__<avr.prog.mk>__)
__<avr.prog.mk>__:

OBJS?=		${SRCS:R:S/$/.o/g}
CLEANFILES+=	${PROG} ${OBJS}
OBJDIRS+=	${SRCS:H:u}

AVRDUDE?=	avrdude
CU?=		cu
PERL?=		perl
OBJDUMP?=	objdump

.if !empty(LD) && ${LD} == ld
.  undef LD
.endif

.if !empty(SRCS:M*.cpp)
LD?=	${CXX}
.else
LD?=	${CC}
.endif

.if ${TARGET} == avr

.PHONY:	.all.usage upload tty

all:	.all.usage

.all.usage: ${PROG}
	${OBJDUMP} -h ${.ALLSRC} | \
	${PERL} ${.CURDIR}/script/show_usage \
		${AVAIL_FLASH} ${AVAIL_SRAM} ${AVAIL_EEPROM}

upload:	all
	${AVRDUDE} -p${AVRDUDE_MCU} -c${AVRDUDE_PROG} -P${AVRDUDE_TTY} \
		-U ${PROG}

tty:	upload
	${CU} -l${AVRDUDE_TTY} -s${TTY_SPEED}

.else

all:	${PROG}

.endif

.include "avr.obj.mk"
.include "avr.dep.mk"
.include "avr.rules.mk"

${PROG}: ${OBJS}
	${LD} ${LDFLAGS} -o ${.TARGET} ${.ALLSRC} ${LIBS}

.endif
