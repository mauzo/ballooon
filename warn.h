/* Warn.h
 * Logging functions.
 */

#ifndef __WARN_H
#define __WARN_H

#define WPANIC  0
#define WERROR  1
#define WWARN   2
#define WNOTICE 3
#define WLOG    4
#define WDEBUG  5
#define WDUMP   6

EXT_C void  warn            (byte level, const char *msg);
EXT_C void  warnf           (byte level, const char *fmt, ...);
EXT_C void  warnx           (byte level, const char *msg);

#endif
