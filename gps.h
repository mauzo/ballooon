/* gps.h
 * Functions for talking to the GPS unit.
 */

#ifndef __GPS_H
#define __GPS_H

#define GPS_ADDR    0x42

// Define structure class for GPS data
struct GPS_DATA {
    byte    Hr;
    byte    Min;
    byte    Sec;
    long    Lat;
    long    Lon;
    long    Alt; // (Above mean sea level)
    byte    numSats;
    byte    fixType;
    byte    Valid;
};

void        checkForLock    (void);
void        parseUBX        (void);
void        printGPSData    (void);

void        gps_setup       (void);
void        gps_loop        (void);

#endif
