# This is a BSD makefile. 
# It requires BSD make (bmake), not GNU make (gmake).

.include "mk/avr.mk"
.sinclude "config.mk"

.if ${TARGET} != avr
AMD=		1
CFLAGS+=	-DAMD -I${.CURDIR}/amd64
.PATH:		${.CURDIR}/amd64

libCore_SRCS=	main.c amd64.c Print.cpp Stream.cpp
.endif

USE_ARDUINO=	Wire Core

PROG=		ballooon
SRCS=		ballooon.ino gps.cpp ubx.cpp warn.cpp

CFLAGS+=	-ffunction-sections -fdata-sections -Os -std=gnu99
CFLAGS+=	-Wall -Wno-parentheses
CFLAGS+=	-I${.CURDIR}

CXXFLAGS=	${CFLAGS:N-std=*} -fno-exceptions
LDFLAGS+=	-Os -Wl,--gc-sections
LIBS+=		-lm

.ifdef AMD
LIBS+=		-lrt
.endif

.include "avr.prog.mk"
.include "avr.arduino.mk"
