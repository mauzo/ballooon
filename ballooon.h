/* ballooon.h
 * Include all the necessary header files.
 */

#ifndef __BALLOOON_H
#define __BALLOOON_H

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
#  include <Arduino.h>
#  include <Wire.h>
#  define EXT_C extern "C"
#else
#  define EXT_C
typedef uint8_t byte;
typedef const __flash char __FlashStringHelper;
#  define F(s) ({ \
    static const __flash char __F[] = s; \
    (__FlashStringHelper *)&__F; \
})
#endif

#include "pad.h"

#include "debug.h"
#include "task.h"
#include "panic.h"

#include "gps.h"
#include "ubx_pkt.h"
#include "ubx.h"

#endif
