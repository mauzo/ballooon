.ifdef USE_ARDUINO

.if empty(ARDUINO_DIR) || !exists(${ARDUINO_DIR})
.  error ARDUINO_DIR must be set!
.endif

libCore_SRCS=	wiring.c wiring_digital.c wiring_analog.c wiring_pulse.c \
		wiring_shift.c \
		main.cpp new.cpp \
		WString.cpp WMath.cpp WInterrupts.c \
		Print.cpp Stream.cpp HardwareSerial.cpp \
		USBCore.cpp CDC.cpp HID.cpp \
		Tone.cpp IPAddress.cpp

libCore_DIRS=	${ARDUINO_DIR}/hardware/arduino/cores/arduino \
		${ARDUINO_DIR}/hardware/arduino/variants/standard

libWire_SRCS=	Wire.cpp twi.c

.for u in ${USE_ARDUINO}
.  if defined(lib${u}_DIRS)
__ARD_LIB_DIRS+=	${lib${u}_DIRS}
.  else
__ARD_LIB_DIRS+=	${ARDUINO_DIR}/libraries/${u} \
			${ARDUINO_DIR}/libraries/${u}/utility
.  endif
.endfor

.PATH:		${__ARD_LIB_DIRS}
CFLAGS+=	${__ARD_LIB_DIRS:S/^/-I/}

__ARD_LIBS=	${USE_ARDUINO:S/^/lib/}
OBJDIRS+=	${__ARD_LIBS}
ARDUINO_LIBS=	${__ARD_LIBS:S/$/.a/}
CLEANFILES+=	${ARDUINO_LIBS}

.for l in ${__ARD_LIBS}
.  for s in ${${l}_SRCS}
CLEANFILES+=	${l}/${s:R}.o ${l}/${s}
.  endfor
.endfor

CLEANFILES+=	${SRCS:M*.ino:S/.ino$/.cpp/}

.SUFFIXES: .ino

.ino.cpp:
	echo "#include <Arduino.h>" >${.TARGET}
	cat ${.IMPSRC} >>${.TARGET}

.ifdef PROG
${PROG}: ${ARDUINO_LIBS}
.endif

.for l in ${__ARD_LIBS}
.  for s in ${${l}_SRCS}

${l}.a: ${l}/${s:R}.o

${l}/${s}: ${s}
	ln -sf ${${s}:P} ${.TARGET}
.  endfor

${l}.a:
	${AR} rcs ${.TARGET} ${.ALLSRC}
.endfor

.endif
