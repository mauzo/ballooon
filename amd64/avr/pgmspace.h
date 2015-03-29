#ifndef __AVR_PGMSPACE_H
#define __AVR_PGMSPACE_H

#define PROGMEM
#define __flash

#define pgm_read_byte(p) (*(unsigned char *)p)

#define vsnprintf_P     vsnprintf
#define memcpy_P        memcpy
#define strlcpy_P       strlcpy

#endif
