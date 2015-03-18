/* gps.h
 * Definitions for talking to the GPS unit over I2C
 */

#ifndef __GPS_H
#define __GPS_H

typedef struct gps_fix {
    byte    hr;
    byte    min;
    byte    sec;

    long    lat;
    long    lon;
    long    alt;

    /* the millis() time we got this fix */
    long    when;
    /* the number of satellites we locked on to */
    byte    nsats;
    /* the fix type: UBX_NAVPVT_FIX_* */
    byte    fix_type;
} gps_fix;

extern gps_fix  gps_last_fix;
extern task     gps_task;

#endif
