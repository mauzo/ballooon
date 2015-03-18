/* cxxcompat.h
 * Compatibility stuff for Arduino C++ dependencies.
 *
 * The Arduino API headers are in C++, so they can't be used from C.
 * This file has C equivalents for the bits we need.
 */

#ifndef __CXXCOMPAT_H
#define __CXXCOMPAT_H

typedef uint8_t byte;

typedef const __flash char __FlashStringHelper;
#define F(s) ({ \
    static const __flash char __F[] = s; \
    (__FlashStringHelper *)&__F; \
})

/* in wiring.c */
unsigned long   millis      (void);

#endif
