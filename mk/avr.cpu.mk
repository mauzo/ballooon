.if !target(__<avr.cpu.mk>__)
__<avr.cpu.mk>__:

.if ${TARGET} == avr

MACHINE=	avr

CC=		avr-gcc
CXX=		avr-g++
OBJCOPY=	avr-objcopy
AR=		avr-ar
OBJDUMP=	avr-objdump

CFLAGS=		-mmcu=${AVR_MCU} -DF_CPU=16000000L -DARDUINO=105
LDFLAGS=	-mmcu=${AVR_MCU}

.endif

.endif
