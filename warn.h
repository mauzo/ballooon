/* Warn.h
 * Logging functions.
 */

#ifndef __WARN_H
#define __WARN_H

#include <avr/pgmspace.h>

#define WPANIC  0
#define WERROR  1
#define WWARN   2
#define WNOTICE 3
#define WLOG    4
#define WDEBUG  5
#define WDUMP   6

#define WARN_PGM    0x1
#define WARN_FMT    0x2
#define WARN_STMP   0x4
#define WARN_NL     0x8

#define sF(x) ({ \
    static const char __sF[] PROGMEM = "" x; \
    &__sF[0]; \
})

/* These macros take a string literal argument, and make sure it ends up
 * stored in flash.
 */

/* XXX gcc-specific ,##__VA_ARGS__ */
#define warn(l, m) \
    warn_flags(WARN_PGM|WARN_STMP|WARN_NL, (l), sF(m))
#define warnf(l, m, ...) \
    warn_flags(WARN_PGM|WARN_FMT|WARN_STMP|WARN_NL, (l), sF(m), ## __VA_ARGS__)

void    warn_flags  (byte flags, byte level, const char *fmt, ...);

#endif
