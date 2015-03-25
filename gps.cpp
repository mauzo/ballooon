/* gps.cpp
 * Functions for talking to the GPS module.
 */

#include <Arduino.h>

#include "ubx.h"
#include "gps.h"

byte setIOtoUBX[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x96}; 
byte airborne1g[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF,
    0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00,
    0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC};
// 0x10 0x27 0x00 0x00 for 'fixed altitude variance? Time and pos masks?
byte reqNAV_PVT[] =  {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19};

static char             txString[100];
static GPS_DATA         gpsData;
static GPS_DATA         lastKnownFix;
static boolean          gpsLock         = false;
static unsigned long    gpsCheckTime;
static byte             UBXbuffer[UBX_MAX_PAYLOAD];
static byte             UBXclass;
static byte             UBXid;

static long     join4Bytes  (byte *data);

void
gps_loop (void)
{
    if(millis() > gpsCheckTime) {
        Serial.println("Requesting NAV-PVT.");
        if(!sendUBX(GPS_ADDR, reqNAV_PVT, sizeof(reqNAV_PVT)))
            Serial.println("Error sending command.");
        getGPSData(GPS_ADDR);
        checkForLock();
        gpsCheckTime += 10000;
        printGPSData();
    }
}

void
gps_setup (void)
{
    Serial.println("Setting IO protocols.");
    if(!sendUBX(GPS_ADDR, setIOtoUBX, sizeof(setIOtoUBX)))
        Serial.println("Error sending command.");
    
    Serial.println("Awaiting confirmation.");
    if(!getUBX_ACK(GPS_ADDR, setIOtoUBX))
        Serial.println("Error: no confirmation received for port IO command.");
    
    Serial.println("Setting flight mode (1g).");
    if(!sendUBX(GPS_ADDR, airborne1g, sizeof(airborne1g)))
        Serial.println("Error sending command.");
    
    Serial.println("Awaiting confirmation.");
    if(!getUBX_ACK(GPS_ADDR, airborne1g))
        Serial.println("Error: no confirmation received for port IO command.");

    gpsCheckTime = millis();
}

void 
printGPSData (void) 
{
    Serial.println("\nGPS Data:");
    sprintf(txString, "Time: %02u:%02u:%02u", 
        gpsData.Hr, gpsData.Min, gpsData.Sec);
    Serial.println(txString);
    sprintf(txString, "Lat: %li, Lon: %li", gpsData.Lat, gpsData.Lon);
    Serial.println(txString);
    sprintf(txString, "Altitude: %li", gpsData.Alt);
    Serial.println(txString);
    sprintf(txString, "Number of satellites used: %u", gpsData.numSats);
    Serial.println(txString);
    sprintf(txString, "Type of lock: %u\n", gpsData.fixType);
    Serial.println(txString);
}

// Good fix is > 4 sats and fixType = 3 or 4.
void 
checkForLock (void)
{
    if(((int)gpsData.fixType == 3 || 
        (int)gpsData.fixType == 4) && (int)gpsData.numSats > 4
    ) {
        if(gpsLock == false)
            Serial.println("*****Lock acquired*****");
        gpsLock         = true;
        lastKnownFix    = gpsData;
    }
    else {
        if(gpsLock == true)
            Serial.println("*****Lock lost*****");
        gpsLock = false;
    }
}

// Chops out only the data I want from the UBX sentence
void 
parseUBX (void)
{
    if(UBXclass == 0x01 && UBXid == 0x07) {
        gpsData.Hr      = UBXbuffer[8];
        gpsData.Min     = UBXbuffer[9];
        gpsData.Sec     = UBXbuffer[10];
        gpsData.Valid   = UBXbuffer[11];
        gpsData.fixType = UBXbuffer[20];
        gpsData.numSats = UBXbuffer[23];
        gpsData.Lon     = join4Bytes(&UBXbuffer[24]) / 100;
        gpsData.Lat     = join4Bytes(&UBXbuffer[28]) / 100;
        gpsData.Alt     = join4Bytes(&UBXbuffer[36]) / 100;
    }
}

// Joins 4 bytes into one long. Pointers!
static long 
join4Bytes (byte *data)
{
    union long_union {
        int32_t dword;
        uint8_t  byte[4];
    } longUnion;

    longUnion.byte[0] = *data;
    longUnion.byte[1] = *(data+1);
    longUnion.byte[2] = *(data+2);
    longUnion.byte[3] = *(data+3);

    return(longUnion.dword);
}

