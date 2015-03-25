.ifndef AMD

MACHINE=	avr

CC=		avr-gcc
CXX=		avr-g++
OBJCOPY=	avr-objcopy
AR=		avr-ar

CFLAGS=		-mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=105
LDFLAGS=	-mmcu=atmega328p

.endif
