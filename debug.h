/* debug.h
 * Debugging functions.
 */

#ifndef __DEBUG_H
#define __DEBUG_H

void        debug_setup     (void);
EXT_C void  warn            (const char *msg);
EXT_C void  warnf           (const char *fmt, ...);
EXT_C void  warnx           (const char *msg);

#endif
