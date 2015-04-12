# This is a BSD makefile. 
# It requires BSD make (bmake), not GNU make (gmake).

.sinclude "config.mk"
.include "mk/avr.mk"

USE_ARDUINO=	Wire SdFat Core

AVRDUDE_MCU= 	m328p
AVRDUDE_PROG=	arduino
TTY_SPEED=	9600

.if ${TARGET} != avr
.PATH:		${.CURDIR}/host
CFLAGS+=	-DHOST -I${.CURDIR}/host
libCore_SRCS=	main.c host.c Print.cpp Stream.cpp
.endif

libSdFat_DIRS=	${.CURDIR}/SdFat/SdFat ${.CURDIR}/SdFat/SdFat/utility
libSdFat_SRCS=	MinimumSerial.cpp SdFatBase.cpp SdFatUtil.cpp \
		SdSpiCard.cpp SdSpiSAM3X.cpp SdSpiSTM32F1.cpp SdSpiTeensy3.cpp \
		FatFile.cpp FatFileLFN.cpp FatFilePrint.cpp FatFileSFN.cpp FatVolume.cpp \
		FmtNumber.cpp fstream.cpp istream.cpp ostream.cpp StdioStream.cpp

PROG=		ballooon
SRCS=		ballooon.ino sd.cpp warn.cpp
		#camera.cpp gps.cpp rtty.cpp ubx.cpp

CFLAGS+=	-ffunction-sections -fdata-sections -Os -std=gnu99
CFLAGS+=	-Wall -Wno-parentheses
CFLAGS+=	-I${.CURDIR}

CXXFLAGS=	${CFLAGS:N-std=*} -fno-exceptions
LDFLAGS+=	-Os -Wl,--gc-sections
LIBS+=		-lm

.include "avr.prog.mk"
.include "avr.arduino.mk"
