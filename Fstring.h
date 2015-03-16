/* Fstring.h
 * Functions for dealing with strings in flash.
 */

#ifndef __FSTRING_H
#define __FSTRING_H

#define FSTRSIZ 81

/* A pointer to a string in flash */
typedef const __FlashStringHelper *Fstr;

/* copy a string out of flash into the buffer */
char *      strFdup     (Fstr src);

#define vsnprintf   __builtin_vsnprintf

/* sprintf into a fixed static buffer */
char *      bprintf     (const char *fmt, ...);
char *      vbprintf    (const char *fmt, va_list ap);

/* bprintf with a format in flash */
char *      Fprintf     (Fstr fmt, ...);
char *      vFprintf    (Fstr fmt, va_list ap);

#endif
