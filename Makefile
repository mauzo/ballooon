# This is a BSD makefile. 
# It requires BSD make (bmake), not GNU make (gmake).

.include "mk/avr.mk"

ARDUINO_DIR=	/usr/local/arduino
USE_ARDUINO=	Wire

LD=		${CXX}

PROG=		ballooon
SRCS=		ballooon.ino gps.cpp ubx.cpp

.ifdef AMD
SRCS+=	${AMD_SRCS:S!^!amd64/!}
.else
SRCS+=	${AVR_SRCS}
.endif

OBJDIRS+=	${SRCS:H:u}

OBJS=		${SRCS:R:S/$/.o/g}
CLEANFILES+=	${PROG} ${OBJS}

CFLAGS+=	-ffunction-sections -fdata-sections -Os -std=gnu99
CFLAGS+=	-Wall -Wno-parentheses
CFLAGS+=	-I${.CURDIR}

.ifdef AMD
CFLAGS+=	-g
CFLAGS+=	-I${.CURDIR}/amd64
.endif

CXXFLAGS=	${CFLAGS:N-std=*} -fno-exceptions
LDFLAGS+=	-Os -Wl,--gc-sections
LIBS+=		-lm -lc -lgcc

.include "avr.arduino.mk"
.include "avr.obj.mk"
.include "avr.dep.mk"

.SUFFIXES: .c .cpp .ino .o

.PHONY: all

.ifdef AMD
all:	${PROG}
.else
all:	${PROG}.hex ${PROG}.eep
.endif

${PROG}: ${OBJS} ${ARDUINO_LIBS}
	${LD} ${LDFLAGS} -o ${.TARGET} ${.ALLSRC} ${LIBS}

${PROG}.hex: ${PROG}
	${OBJCOPY} -O ihex -R .eeprom ${PROG} ${.TARGET}

${PROG}.eep: ${PROG}
	${OBJCOPY} -O ihex -j .eeprom ${PROG} ${.TARGET}

.c.o:
	${CC} ${CFLAGS} -c -o ${.TARGET} ${.IMPSRC}

.cpp.o:
	${CXX} ${CXXFLAGS} -c -o ${.TARGET} ${.IMPSRC}

.ino.cpp:
	echo "#include <Arduino.h>" >${.TARGET}
	cat ${.IMPSRC} >>${.TARGET}
