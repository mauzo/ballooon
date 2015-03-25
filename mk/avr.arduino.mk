.if !exists(${ARDUINO_DIR})
.  error ARDUINO_DIR must be set!
.endif

libCore_SRCS+=	wiring.c wiring_digital.c wiring_analog.c wiring_pulse.c \
		wiring_shift.c \
		main.cpp new.cpp \
		WString.cpp WMath.cpp WInterrupts.c \
		Print.cpp Stream.cpp HardwareSerial.cpp \
		USBCore.cpp CDC.cpp HID.cpp \
		Tone.cpp IPAddress.cpp

libWire_SRCS+=	Wire.cpp twi.c

__ARD_LIBS=	${USE_ARDUINO:S!^!${ARDUINO_DIR}/libraries/!}

.for d in ${ARDUINO_DIR}/hardware/arduino/cores/arduino \
	${ARDUINO_DIR}/hardware/arduino/variants/standard \
	${__ARD_LIBS} ${__ARD_LIBS:S!$!/utility!}

.PATH:		${d}
CFLAGS+=	-I${d}
.endfor

.for l in ${USE_ARDUINO:S/^/lib/} libCore
OBJDIRS+=	${l}
ARDUINO_LIBS+=	${l}.a
CLEANFILES+=	${l}.a

.  for s in ${${l}_SRCS}
CLEANFILES+=	${l}/${s:R}.o ${l}/${s}

${l}.a: ${l}/${s:R}.o

${l}/${s}: ${s}
	ln -s ${${s}:P} ${.TARGET}
.  endfor

${l}.a:
	${AR} rcs ${.TARGET} ${.ALLSRC}

.endfor
