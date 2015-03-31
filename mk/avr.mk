.if !target(__<avr.mk>__)
__<avr.mk>__:

.PATH: ${.CURDIR}/mk

TARGET?=	avr
.include "avr.obj.init.mk"

.PHONY: all

.MAIN: all

.include "avr.cpu.mk"

.endif
