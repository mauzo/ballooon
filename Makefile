# This is a BSD makefile. 
# It requires BSD make (bmake), not GNU make (gmake).
#
# You need to create a file config.mk in the source directory containing
# settings for at least the following variables:
# 	ARDUINO_DIR	The path to the Arduino library installation.
#	AVRDUDE_TTY	The tty device to upload to the AVR.
#
# Available targets (bmake <foo>):
# 	depend		Rebuild dependency information.
# 	all		Build the program.
# 	upload		Upload the program to the AVR.
# 	tty		Upload and connect to the AVR serial port.
# 	clean		Remove the build files.
# 	cleandir	Remove the whole build directory.
#
# In a clean checkout, or after adding a source file or a header
# inclusion, run 'make depend'. This ensures bmake knows which files
# depend on which other files, so it can rebuild correctly if a header
# has changed. If the build fails, try 'clean' or 'cleandir' to see if
# that fixes the problem (run 'depend' again after 'cleandir').
#
# Passing TARGET=amd64 will attempt to build the system to run on the
# local host. The generated binary will be in obj.amd64 in the source
# directory.
#

.sinclude "config.mk"
.include "mk/avr.mk"

# What to build

PROG=		ballooon
SRCS=		ballooon.ino camera.cpp gps.cpp rtty.cpp ubx.cpp warn.cpp

USE_ARDUINO=	Wire Core
LIBS+=		-lm -lprintf_flt

# Information about the machine we are building for

AVAIL_FLASH=	32768
AVAIL_SRAM=	2048
AVAIL_EEPROM=	1024

AVRDUDE_MCU= 	m328p
AVRDUDE_PROG=	arduino
TTY_SPEED=	9600

# Compiler and linker flags

CFLAGS+=	-ffunction-sections -fdata-sections -Os -std=gnu99
CFLAGS+=	-Wall -Wno-parentheses
CFLAGS+=	-I${.CURDIR}

CXXFLAGS=	${CFLAGS:N-std=*} -fno-exceptions
LDFLAGS+=	-Os -Wl,--gc-sections -T${.CURDIR}/script/ldscript

# Host-build compat

.if ${TARGET} != avr
.PATH:		${.CURDIR}/host
CFLAGS+=	-DHOST -I${.CURDIR}/host
libCore_SRCS=	main.c host.c Print.cpp Stream.cpp
.endif

.include "avr.prog.mk"
.include "avr.arduino.mk"
