.PATH: ${.CURDIR}/mk

.PHONY: all

.MAIN: all

TARGET?=	avr
MAKEOBJDIR?=	${.CURDIR}/obj.${TARGET}

.if !make(obj) && !exists(${MAKEOBJDIR})
BROKEN=		Run 'make obj' first!
.else
.OBJDIR:	${MAKEOBJDIR}
.endif

.include "avr.cpu.mk"
