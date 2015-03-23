/* ballooon.h
 * Include all the necessary header files.
 */

#ifndef __BALLOOON_H
#define __BALLOOON_H

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __AVR__
#  include <avr/pgmspace.h>
#  include <Arduino.h>
#  define WW(f, ...) (void)0
#else
#  include "avr-compat.h"
#  define WW(f, ...) fprintf(stderr, f, ##__VA_ARGS__)
#endif

#define WP(m, p) \
    WW("%s [%lx] -> [%02x %02x %02x %02x]\n", \
        (m), (unsigned long)p, \
        (unsigned)dF((byte*)p + 0), \
        (unsigned)dF((byte*)p + 1), \
        (unsigned)dF((byte*)p + 2), \
        (unsigned)dF((byte*)p + 3))

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
