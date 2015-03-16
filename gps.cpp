// gps.cpp
// Functions for talking to the GPS unit over I2C
//

#include "ballooon.h"

#define GPS_ADDR        0x42    // I2C address for uBlox Max7-q
#define UBX_MAX_PAYLOAD 100     // set to maximum message you want to receive
#define UBX_SYNC_CHAR1  0xB5
#define UBX_SYNC_CHAR2  0x62

static void     gps_setup       (void);
static void     gps_run         (void);
static void     gps_reset       (void);

task gps_task = {
    .name       = "GPS",
    .active     = 0,
    .when       = 0,

    .setup      = gps_setup,
    .run        = gps_run,
    .reset      = gps_reset
};

static void     UBXchecksum     (unsigned char data);
static void     checkForLock    (void);
static boolean  getGPSData      (void);
static boolean  getUBX_ACK      (uint8_t *MSG);
static long     join4Bytes      (unsigned char *data);
static void     parseUBX        (void);
static void     printGPSData    (void);
static boolean  sendUBX         (uint8_t *MSG, uint8_t len);

static unsigned char setIOtoUBX[] = {
    0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xA0, 0x96
};
static unsigned char airborne1g[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 
    0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 
    0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 
    0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x16, 0xDC
};
// 0x10 0x27 0x00 0x00 for 'fixed altitude variance? Time and pos masks?
static unsigned char reqNAV_PVT[] = {
    0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19
};

static char txString[100];

struct GPS_DATA {
  unsigned char Hr;
  unsigned char Min;
  unsigned char Sec;
  long Lat;
  long Lon;
  long Alt; // (Above mean sea level)
  unsigned char numSats;
  unsigned char fixType;
  unsigned char Valid;
};

static GPS_DATA gpsData;
static GPS_DATA lastKnownFix;
static boolean gpsLock = false;

// Variables that are filled by NAV-PVT:
// (How many of these really need to be global? Move some to getGPSData())
static byte UBXstate = 0;
static unsigned char Checksum_A = 0;
static unsigned char Checksum_B = 0;
static unsigned char UBXclass;
static unsigned char UBXid;
static unsigned char UBXlengthLSB;
static unsigned char UBXlengthMSB;
static byte UBXlength;
static unsigned char UBXpayloadIdx;
static unsigned char UBXbuffer[UBX_MAX_PAYLOAD];
static unsigned char UBXckA;
static unsigned char UBXckB;

static void
gps_setup (void)
{
    Wire.begin(); // Start I2C link to GPS

    warn("Setting IO protocols.");
    if (!sendUBX(setIOtoUBX, sizeof(setIOtoUBX)))
        panic("Cannot set GPS unit to UBX");

    warn("Awaiting confirmation.");
    if (!getUBX_ACK(setIOtoUBX))
        panic("No ACK setting GPS unit to UBX");

    warn("Setting flight mode (1g).");
    if (!sendUBX(airborne1g, sizeof(airborne1g)))
        panic("Cannot set GPS unit to flight mode");

    warn("Awaiting confirmation.");
    if (!getUBX_ACK(airborne1g))
        panic("No ACK setting GPS unit to flight mode");

    gps_task.active = 1;
}

static void
gps_run (void)
{
    warn("Requesting NAV-PVT.");
    if (!sendUBX(reqNAV_PVT, sizeof(reqNAV_PVT)))
        panic("Cannot sent NAV-PVT to GOS unit");
    getGPSData();
    checkForLock();
    printGPSData(); // This would be replaced by a txSentence function.

    gps_task.when = millis() + 10000;
}

static void
gps_reset (void)
{
    warn("XXX RESET GPS UNIT");
}

static void 
UBXchecksum (unsigned char data)
{
    Checksum_A += data;
    Checksum_B += Checksum_A;
}

static void
checkForLock (void)
{ 
    // Good fix is > 4 sats and fixType = 3 or 4.
    if ((gpsData.fixType == 3 || gpsData.fixType == 4) 
        && gpsData.numSats > 4
    ) {
        if (!gpsLock)
            warn("*****Lock acquired*****");
        gpsLock = true;
        lastKnownFix = gpsData;
    }
    else {
        if (gpsLock)
            warn("*****Lock lost*****");
        gpsLock = false;
    }
}

static boolean
getGPSData (void)
{
    long    timeoutTime = millis() + 3000;
    boolean timeout = false;
    boolean EOM = false;

    while (!EOM && !timeout) {
	if (millis() > timeoutTime) {
	    timeout = true;
	    panic("getGPSData timed out");
	    return false;
	}
        // If there is no data available...
	if (!Wire.available()) {
	    // ...ask for some
	    Wire.requestFrom(GPS_ADDR, BUFFER_LENGTH);
        }
        else {
            // Wire.available() is true, so read the data
            unsigned char   tempChar;

            while (Wire.available() && !EOM) {
                tempChar = Wire.read();

                switch (UBXstate) {
                case 0:         // Awaiting Sync Char 1
                    if (tempChar == UBX_SYNC_CHAR1) //B5
                        UBXstate++;
                    break;
                case 1:         // Awauting Sync Char 2
                    if (tempChar == UBX_SYNC_CHAR2) //62
                        UBXstate++;
                    else
                        UBXstate = 0;
                    break;
                case 2:         // Awaiting Class
                    UBXclass = tempChar;
                    UBXchecksum(UBXclass);
                    UBXstate++;
                    break;
                case 3:         // Awaiting Id
                    UBXid = tempChar;
                    UBXchecksum(UBXid);
                    UBXstate++;
                    break;
                case 4:         // Awaiting Length LSB
                                // (little endian so LSB is first)
                    UBXlengthLSB = tempChar;
                    UBXchecksum(UBXlengthLSB);
                    UBXstate++;
                    break;
                case 5:         // Awaiting Length MSB
                    UBXlengthMSB = tempChar;
                    UBXchecksum(UBXlengthMSB);
                    UBXstate++;
                    UBXpayloadIdx = 0;

                    // Convert little endian MSB & LSB into integer
                    UBXlength = (byte) (UBXlengthMSB << 8) | UBXlengthLSB;
                    if (UBXlength >= UBX_MAX_PAYLOAD) {
                        panic("UBX payload length too large (>100)");
                        UBXstate = 0;
                        //  Bad data received so reset and
                        Checksum_A = 0;
                        Checksum_B = 0;
                        return false;
                    }
                    break;
                case 6:
                    UBXbuffer[UBXpayloadIdx] = tempChar;
                    UBXchecksum(tempChar);
                    UBXpayloadIdx++;
                    if (UBXpayloadIdx == UBXlength)
                        // Just processed last byte of payload, so move on
                        UBXstate++;
                    break;
                case 7:         // Awaiting Checksum 1
                    UBXckA = tempChar;
                    UBXstate++;
                    break;
                case 8:         // Awaiting Checksum 2
                    UBXckB = tempChar;
                    // Check the calculated checksums match actual checksums
                    if ((Checksum_A == UBXckA) && (Checksum_B == UBXckB)) {
                        // Checksum is good so parse the message
                        parseUBX();
                        EOM = true;
                    } 
                    else {
                        panic("UBX PAYLOAD BAD CHECKSUM!!");
                        return false;
                    }

                    // Start again at 0
                    UBXstate = 0;
                    Checksum_A = 0;
                    Checksum_B = 0;
                    break;
                }
            }
        }
    }

    // We 're at the end, which must mean everything' s ok.
    return true;
}

// Calculate expected UBX ACK packet and parse UBX response from GPS
static boolean
getUBX_ACK (uint8_t *MSG) 
{
    uint8_t         readByte;
    uint8_t         ackByteID = 0;
    uint8_t         ackPacket[10];
    unsigned long   startTime = millis();

    warn("Reading ACK response: ");

    // Construct the expected ACK packet    
    ackPacket[0] = 0xB5;    // header
    ackPacket[1] = 0x62;    // header
    ackPacket[2] = 0x05;    // class
    ackPacket[3] = 0x01;    // id
    ackPacket[4] = 0x02;    // length1
    ackPacket[5] = 0x00;    // length2
    ackPacket[6] = MSG[2];  // ACK class
    ackPacket[7] = MSG[3];  // ACK id
    ackPacket[8] = 0;	    // CK_A
    ackPacket[9] = 0;	    // CK_B

    // Calculate the first part of the checksum
    for (uint8_t i=2; i<8; i++) {
        ackPacket[8] = ackPacket[8] + ackPacket[i];
        ackPacket[9] = ackPacket[9] + ackPacket[8];
    }

    while (1) {
        // Timeout if no valid response in 3 seconds
        if (millis() - startTime > 3000)
            panic("Reading ACK from GPS timed out");

        // Make sure data is available to read
        // Request 32 bytes from GPS
        Wire.requestFrom(GPS_ADDR, BUFFER_LENGTH);

        while (Wire.available()) {
            readByte = Wire.read();
            
            // i.e. that there is actually something to read
            if (readByte != 0xFF) {
                // Check that bytes arrive in sequence as per expected 
                // ACK packet
                if (readByte == ackPacket[ackByteID]) { 
                    ackByteID++;
                    Serial.print(readByte, HEX);
                    
                    // Test for success
                    if (ackByteID > 9) {
                        // All packets in order!
                        warn(" (SUCCESS!)");
                        return true;
                    }
                }
                else {
    	      // Reset and look again, invalid order
                  ackByteID = 0;
                }
            } 
        }
    }
}

// Joins 4 bytes into one long. Pointers!
static long
join4Bytes (unsigned char *data)
{
    union long_union {
        int32_t dword;
        uint8_t  byte[4];
    } longUnion;

    longUnion.byte[0] = *data;
    longUnion.byte[1] = *(data+1);
    longUnion.byte[2] = *(data+2);
    longUnion.byte[3] = *(data+3);

    return longUnion.dword;
}

// Chops out only the data I want from the UBX sentence
static void 
parseUBX (void)
{
    if (UBXclass == 0x01 && UBXid == 0x07)
    {
        gpsData.Hr = UBXbuffer[8];
        gpsData.Min = UBXbuffer[9];
        gpsData.Sec = UBXbuffer[10];
        gpsData.Valid = UBXbuffer[11];
        gpsData.fixType = UBXbuffer[20];
        gpsData.numSats = UBXbuffer[23];
        gpsData.Lon = join4Bytes(&UBXbuffer[24]) / 100;
        gpsData.Lat = join4Bytes(&UBXbuffer[28]) / 100;
        gpsData.Alt = join4Bytes(&UBXbuffer[36]) / 100;
    }
}

static void
printGPSData (void)
{
    warn("\nGPS Data:");
    warn("Time: %02u:%02u:%02u", gpsData.Hr, gpsData.Min, gpsData.Sec);
    warn("Lat: %li, Lon: %li", gpsData.Lat, gpsData.Lon);
    warn("Altitude: %li", gpsData.Alt);
    warn("Number of satellites used: %u", gpsData.numSats);
    warn("Type of lock: %u\n", gpsData.fixType);
}

// Send a byte array of UBX protocol to the GPS
static boolean
sendUBX (uint8_t *MSG, uint8_t len)
{
    boolean result;

    // Start I2C transmission
    Wire.beginTransmission(GPS_ADDR);

    // Write one byte at a time
    for(int i=0; i<len; i++) {
        // Wire library bug. Detect 32 bytes (defined in Wire.h) 
        // and start again.
        if (i > 0 && i % BUFFER_LENGTH == 0) {
            if (Wire.endTransmission() != 0) {
                panic("Error in Wire.endTransmission at buffer=max");
                return result;
            }
            Wire.beginTransmission(GPS_ADDR);
        }

        Serial.print(MSG[i], HEX);
        if (Wire.write(MSG[i]) != 1) {
            panic("Error in Wire.write");
            return result;
        }
    }

    if (Wire.endTransmission() != 0) {
        panic("Error in Wire.endTransmission");
        return result;
    }

    warn("Success!");
    // Reached the end, no problems encountered
    return true;
}
