/* debug.h
 * Debugging functions.
 */

#ifndef __DEBUG_H
#define __DEBUG_H

void    debug_setup     (void);
void    warn            (Fstr msg);
void    warnf           (Fstr fmt, ...);
void    warnx           (const char *msg);

#endif
