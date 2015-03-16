/* gps.h
 * Definitions for talking to the GPS unit over I2C
 */

#ifndef __GPS_H
#define __GPS_H

extern long     gpsCheckTime;

void    gps_check       (void);
void    gps_setup       (void);

#endif
