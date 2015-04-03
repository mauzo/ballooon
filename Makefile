# This is a BSD makefile. 
# It requires BSD make (bmake), not GNU make (gmake).

CC=	gcc48
CXX=	g++48

.include "mk/avr.mk"

ARDUINO_DIR=	/usr/local/arduino

USE_ARDUINO=	Wire Core

.if ${TARGET} != avr
.PATH:		${.CURDIR}/host
CFLAGS+=	-DHOST -I${.CURDIR}/host
libCore_SRCS=	main.c host.c Print.cpp Stream.cpp
.endif

PROG=		ballooon
SRCS=		ballooon.ino camera.cpp gps.cpp ubx.cpp warn.cpp

CFLAGS+=	-ffunction-sections -fdata-sections -Os -std=gnu99
CFLAGS+=	-Wall -Wno-parentheses
CFLAGS+=	-I${.CURDIR}

CXXFLAGS=	${CFLAGS:N-std=*} -fno-exceptions
LDFLAGS+=	-Os -Wl,--gc-sections
LIBS+=		-lm

.include "avr.prog.mk"
.include "avr.arduino.mk"
