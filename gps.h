/* gps.h
 * Functions for talking to the GPS unit.
 */

#ifndef __GPS_H
#define __GPS_H

#include "task.h"

#define GPS_ADDR    0x42

typedef struct {
    byte        hr;
    byte        min;
    byte        sec;

    long        lat;
    long        lon;
    long        alt; // (Above mean sea level)

    byte        num_sat;
    byte        fix_type;

    uint32_t    itow;
    long        when;
} gps_data;

/* .when == 0 means we have no fix; the other fields are undefined.
 * .itow == 0 means the fix is stale.
 */
#define GPS_FIX_VALID(g) ((g)->when != 0)
#define GPS_FIX_STALE(g) ((g)->itow == 0)

extern task     gps_task;
extern gps_data gps_last_fix;

#endif
