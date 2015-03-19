/* ballooon.h
 * Include all the necessary header files.
 */

#ifndef __BALLOOON_H
#define __BALLOOON_H

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <avr/pgmspace.h>

#include <Arduino.h>

#ifdef __cplusplus
#  define EXT_C extern "C"
#else
#  define EXT_C
#endif

#include "flash.h"
#include "pad.h"

#include "panic.h"
#include "serial.h"
#include "task.h"
#include "warn.h"

#include "gps.h"
#include "ubx_pkt.h"
#include "ubx.h"

#endif
