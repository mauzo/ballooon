.PATH: ${.CURDIR}/mk

.PHONY: all

.MAIN: all

.include "avr.cpu.mk"
.include "avr.obj.mk"
