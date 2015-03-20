/* serial.h
 * Provide a serial port interface to C.
 */

#ifndef __SERIAL_H
#define __SERIAL_H

void        serial_setup    (void);
EXT_C void  serial_write    (const char *msg);

#endif
