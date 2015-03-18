/* debug.h
 * Debugging functions.
 */

#ifndef __DEBUG_H
#define __DEBUG_H

void        debug_setup     (void);
EXT_C void  warn            (fstr msg);
EXT_C void  warnf           (fstr fmt, ...);
EXT_C void  warnx           (const char *msg);

#endif
