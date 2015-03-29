/* ubx.cpp
 * Functions for talking the UBX protocol.
 */

#include <Arduino.h>
#include <Wire.h>

#include "ubx.h"
#include "gps.h"
#include "warn.h"

/* XXX these have to be global because they're used in gps.c */
byte    UBXclass;
byte    UBXid;
byte    UBXbuffer[UBX_MAX_PAYLOAD];

// Variables that are filled by NAV-PVT:
// (How many of these really need to be global? Move some to getGPSData())
static byte UBXstate    = 0;
static byte Checksum_A  = 0;
static byte Checksum_B  = 0;
static byte UBXlengthLSB;
static byte UBXlengthMSB;
static byte UBXlength;
static byte UBXpayloadIdx;
static byte UBXckA;
static byte UBXckB;

void 
UBXchecksum (byte data)
{
    Checksum_A += data;
    Checksum_B += Checksum_A;
}

boolean 
getGPSData (ubx_addr adr)
{
    unsigned long timeoutTime = millis() + 3000;
    boolean timeout   = false;
    boolean EOM       = false;
    
    while (!EOM && !timeout) {
        if (millis() > timeoutTime) {
            timeout = true;
            warn(WWARN, "getGPSData timed out.");
            return false;
        }
        
        if (!Wire.available()) {
            Wire.requestFrom(adr, BUFFER_LENGTH);
        }
        else {
            byte tempChar;
            
            while (Wire.available() && !EOM) {
                tempChar = Wire.read();
                
                switch(UBXstate) {
                case 0:         // Awaiting Sync Char 1
                    if (tempChar == UBX_SYNC_CHAR1) // B5
                        UBXstate++;
                    break;
                case 1:         // Awauting Sync Char 2
                    if (tempChar == UBX_SYNC_CHAR2) // 62
                        UBXstate++;
                    else
                        // Wrong sequence so start again
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
                    UBXlength = (byte)(UBXlengthMSB << 8) | UBXlengthLSB;
                    if (UBXlength >= UBX_MAX_PAYLOAD) {
                        warn(WWARN, "UBX payload length too large (>100");
                        UBXstate    = 0; // Bad data received so reset and 
                        Checksum_A  = 0;
                        Checksum_B  = 0;
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
                        warn(WERROR, "UBX PAYLOAD BAD CHECKSUM!!");
                        return false;
                    }
                    
                    UBXstate    = 0;    // Start again at 0
                    Checksum_A  = 0;
                    Checksum_B  = 0;
                    break;
                }
            }
        }
    }
    // We're at the end, which must mean everything's ok.
    return true;
}

// Send a byte array of UBX protocol to the GPS
boolean 
sendUBX (ubx_addr adr, byte *msg, byte len)
{
    byte    i;
    boolean result;

    // Start I2C transmission
    Wire.beginTransmission(adr);

    /* print a stamp for the debugging output */
    warn_stamp(WDUMP);

    //Write one byte at a time
    for(i=0; i<len; i++) {
        // Wire library bug. Detect 32 bytes (defined in Wire.h) and
        // start again
        if(i > 0 && i % BUFFER_LENGTH == 0) {
            if(Wire.endTransmission() != 0) {
                warn(WERROR, "Error in Wire.endTransmission at buffer=max.");
                return result;
            }
            
            Wire.beginTransmission(adr);
        }

        warnxf(WDUMP, " %02x", msg[i]);
        if(Wire.write(msg[i]) != 1) {
            warn(WERROR, "Error in Wire.write.");
            return result;
        }
    }
    warn_nl(WDUMP);

    if(Wire.endTransmission() != 0) {
        warn(WERROR, "Error in Wire.endTransmission.");
        return result;
    }

    warn(WDEBUG, "Success!");
    return true;
}

// Calculate expected UBX ACK packet and parse UBX response from GPS
boolean 
getUBX_ACK (ubx_addr adr, byte *msg) 
{
    byte    readByte;
    byte    ackByteID       = 0;
    byte    ackPacket[10];
    byte    i;
    unsigned long startTime = millis();

    warn(WDEBUG, "Reading ACK response");
    
    // Construct the expected ACK packet    
    ackPacket[0] = 0xB5;	// header
    ackPacket[1] = 0x62;	// header
    ackPacket[2] = 0x05;	// class
    ackPacket[3] = 0x01;	// id
    ackPacket[4] = 0x02;	// length1
    ackPacket[5] = 0x00;        // length2
    ackPacket[6] = msg[2];	// ACK class
    ackPacket[7] = msg[3];	// ACK id
    ackPacket[8] = 0;	        // CK_A
    ackPacket[9] = 0;	        // CK_B
    
    // Calculate the first part of the checksum
    for (i=2; i<8; i++) {
        ackPacket[8] = ackPacket[8] + ackPacket[i];
        ackPacket[9] = ackPacket[9] + ackPacket[8];
    }
    
    /* timestamp the hex dump */
    warn_stamp(WDUMP);

    while (1) {
        // Timeout if no valid response in 3 seconds
        if (millis() - startTime > 3000) { 
            warn(WERROR, "Timed out reading UBX ACK");
            return false;
        }
        
        // Make sure data is available to read
        Wire.requestFrom(adr, BUFFER_LENGTH);
        
        while (Wire.available()) {
            readByte = Wire.read();

            // The UBX device sends 0xff when it has nothing left to
            // send, so ignore that.
            if(readByte != 0xFF) {
                // Check that bytes arrive in sequence as per expected
                // ACK packet
                if (readByte == ackPacket[ackByteID]) { 
                    ackByteID++;
                    warnxf(WDUMP, " %02x", readByte);
                    
                    // Test for success
                    if (ackByteID > 9) {
                        // All packets in order!
                        warn_nl(WDUMP);
                        warn(WDEBUG, "Successfully read UBX ACK");
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

void
ubx_setup (void)
{
    Wire.begin(); // Start I2C link to GPS
}


