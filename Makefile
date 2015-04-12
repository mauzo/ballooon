# This is a BSD makefile. 
# It requires BSD make (bmake), not GNU make (gmake).

.sinclude "config.mk"
.include "mk/avr.mk"

USE_ARDUINO=	Wire Core

AVRDUDE_MCU= 	m328p
AVRDUDE_PROG=	arduino
TTY_SPEED=	9600

.if ${TARGET} != avr
.PATH:		${.CURDIR}/host
CFLAGS+=	-DHOST -I${.CURDIR}/host
libCore_SRCS=	main.c host.c Print.cpp Stream.cpp
.endif

PROG=		ballooon
SRCS=		ballooon.ino camera.cpp gps.cpp rtty.cpp ubx.cpp warn.cpp

CFLAGS+=	-ffunction-sections -fdata-sections -Os -std=gnu99
CFLAGS+=	-Wall -Wno-parentheses
CFLAGS+=	-I${.CURDIR}

CXXFLAGS=	${CFLAGS:N-std=*} -fno-exceptions
LDFLAGS+=	-Os -Wl,--gc-sections
LIBS+=		-lm

.include "avr.prog.mk"
.include "avr.arduino.mk"
