# This is a BSD makefile. 
# It requires BSD make (bmake), not GNU make (gmake).

.ifdef AMD
CC=	gcc48
CXX=	g++48

CFLAGS+=	-DAMD
.endif

.include "mk/avr.mk"

ARDUINO_DIR=	/usr/local/arduino

USE_ARDUINO=	${AMD:?AMD64:} Wire ${AMD:?:Core}

libAMD64_SRCS=	main.c amd64.c Print.cpp Stream.cpp
libAMD64_DIRS=	${.CURDIR}/amd64 ${libCore_DIRS}

PROG=		ballooon
SRCS=		ballooon.ino gps.cpp ubx.cpp

CFLAGS+=	-ffunction-sections -fdata-sections -Os -std=gnu99
CFLAGS+=	-Wall -Wno-parentheses
CFLAGS+=	-I${.CURDIR}

CXXFLAGS=	${CFLAGS:N-std=*} -fno-exceptions
LDFLAGS+=	-Os -Wl,--gc-sections
LIBS+=		-lm

.include "avr.prog.mk"
.include "avr.arduino.mk"
