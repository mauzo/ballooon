/* ntx.h
 * Functions for using the NTX radio.
 */

#ifndef __NTX_H
#define __NTX_H

#include "task.h"

#define NTX_BUFSIZ  80

extern task ntx_task;

void    ntx_setup       (void);
byte    ntx_send        (byte *buf, byte len);

#endif
