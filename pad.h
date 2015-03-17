/* pad.h
 * Scratchpad and flash strings.
 */

#ifndef __PAD_H
#define __PAD_H

/* A static buffer for scratchpad use */
#define PADSIZ  81
extern char pad[PADSIZ];

/* A pointer to a string in flash */
typedef const __FlashStringHelper *fstr;

/* These all return the length of the string now in the pad, or 0xFF if
 * the string was too long to fit. (This means PADSIZ has to stay below
 * 255, which we probably want it to anyway.)
 */

/* copy a string out of flash into the pad */
byte        pad_fstr    (fstr src);

/* sprintf into the pad */
byte        pad_form    (const char *fmt, ...);
byte        pad_vform   (const char *fmt, va_list ap);

/* form with the format in flash */
byte        pad_fform   (fstr fmt, ...);
byte        pad_vfform  (fstr fmt, va_list ap);

#endif
