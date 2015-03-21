/* pad.h
 * Scratchpad.
 */

#ifndef __PAD_H
#define __PAD_H

/* A static buffer for scratchpad use */
#define PADSIZ  81
extern char pad[PADSIZ];

/* dump the contents of the pad in hex */
EXT_C void  pad_dump    (const char *msg, char *from, size_t len);

/* These all return the length of the string now in the pad, or 0xFF if
 * the string was too long to fit. (This means PADSIZ has to stay below
 * 255, which we probably want it to anyway.)
 */

/* sprintf into the pad */
EXT_C byte  pad_form    (const char *fmt, ...);
EXT_C byte  pad_vform   (const char *fmt, va_list ap);

#endif
