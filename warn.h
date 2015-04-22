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

#define sF(x) ({ \
    static const char __sF[] PROGMEM = (x); \
    &__sF[0]; \
})

/* The unsuffixed versions of these functions should be passed a
 * literal string, which will be put into flash. The _P versions should
 * be passed a flash (progmem) string. The _R versions should be passed
 * an ordinary string in RAM. 
 */

/* XXX gcc-specific ,##__VA_ARGS__ */
#define warn(l, m)          warn_P((l), sF(m))
#define warnf(l, f, ...)    warnf_P((l), sF(f), ## __VA_ARGS__)
#define warnx(l, m)         warnx_P((l), sF(m))
#define warnxf(l, f, ...)   warnxf_P((l), sF(f), ## __VA_ARGS__)

void    warn_P      (byte level, const char *msg);
void    warn_R      (byte level, const char *msg);
void    warn_nl     (byte level);
void    warn_stamp  (byte level);
void    warnf_P     (byte level, const char *fmt, ...);
void    warnf_R     (byte level, const char *fmt, ...);
void    warnx_P     (byte level, const char *msg);
void    warnx_R     (byte level, const char *msg);
void    warnxf_P    (byte level, const char *fmt, ...);
void    warnxf_R    (byte level, const char *fmt, ...);

#endif
