#include <Wire.h>

#define GPS_ADDR 0x42 // I2C address for uBlox Max7-q
unsigned char setIOtoUBX[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x96}; 
unsigned char airborne1g[]  = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC};
// 0x10 0x27 0x00 0x00 for 'fixed altitude variance? Time and pos masks?
unsigned char reqNAV_PVT[] =  {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19}; //Empty payload NAV-PVT message
const unsigned char UBX_MAX_PAYLOAD = 100;  // set to maximum message you want to receive
const unsigned char UBX_SYNC_CHAR1 = 0xB5;
const unsigned char UBX_SYNC_CHAR2 = 0x62;

char txString[100];

struct GPS_DATA // Define structure class for GPS data
{
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

GPS_DATA gpsData;
GPS_DATA lastKnownFix;
boolean gpsLock = false;

// Variables that are filled by NAV-PVT:
// (How many of these really need to be global? Move some to getGPSData())
byte UBXstate = 0;
unsigned char Checksum_A = 0;
unsigned char Checksum_B = 0;
unsigned char UBXclass;
unsigned char UBXid;
unsigned char UBXlengthLSB;
unsigned char UBXlengthMSB;
byte UBXlength;
unsigned char UBXpayloadIdx;
unsigned char UBXbuffer[UBX_MAX_PAYLOAD];
unsigned char UBXckA;
unsigned char UBXckB;

long gpsCheckTime = millis();

void setup()
{
  Serial.begin(9600); // Start debug output
  Wire.begin(); // Start I2C link to GPS
  
  Serial.println("Setting IO protocols.");
  if(!sendUBX(setIOtoUBX, sizeof(setIOtoUBX)))
  {
    Serial.println("Error sending command.");
  }
  
  Serial.println("Awaiting confirmation.");
  if(!getUBX_ACK(setIOtoUBX))
  {
    Serial.println("Error: no confirmation received for port IO command.");
  }
  
  Serial.println("Setting flight mode (1g).");
  if(!sendUBX(airborne1g, sizeof(airborne1g)))
  {
    Serial.println("Error sending command.");
  }
  
  Serial.println("Awaiting confirmation.");
  if(!getUBX_ACK(airborne1g))
  {
    Serial.println("Error: no confirmation received for port IO command.");
  }
}

void loop()
{ 
  if(millis() > gpsCheckTime)
  {
    Serial.println("Requesting NAV-PVT.");
    if(!sendUBX(reqNAV_PVT, sizeof(reqNAV_PVT)))
    {
      Serial.println("Error sending command.");
    }
    getGPSData();
    checkForLock();
    gpsCheckTime += 10000;
    printGPSData(); // This would be replaced by a txSentence function.
  }
}

void printGPSData()
{
  Serial.println("\nGPS Data:");
  sprintf(txString, "Time: %02u:%02u:%02u", gpsData.Hr, gpsData.Min, gpsData.Sec);
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

void checkForLock()
{ // Good fix is > 4 sats and fixType = 3 or 4.
  if(((int)gpsData.fixType == 3 || (int)gpsData.fixType == 4) && (int)gpsData.numSats > 4)
  {
    if(gpsLock == false)
    {
      Serial.println("*****Lock acquired*****");
    }
    gpsLock = true;
    lastKnownFix = gpsData;
  }
  else
  {
    if(gpsLock == true)
    {
      Serial.println("*****Lock lost*****");
    }
    gpsLock == false;
  }
}

boolean getGPSData()
{
  long timeoutTime = millis() + 3000;
  boolean timeout = false;
  boolean EOM = false;
  
  while(!EOM && !timeout)
  {
    if(millis() > timeoutTime)
    {
      timeout = true;
      Serial.println("getGPSData timed out.");
      return false;
    }
    
    if(!Wire.available()) // If there is no data available...
    {
      Wire.requestFrom(GPS_ADDR, BUFFER_LENGTH); // ...ask for some!
    }
    else // Wire.available() is true, so read the data
    {
      unsigned char tempChar;
      
      while(Wire.available() && !EOM)
      {
        tempChar = Wire.read();
        
        switch(UBXstate)
        {
          case 0:    // Awaiting Sync Char 1
            if (tempChar == UBX_SYNC_CHAR1) // B5
            {
              UBXstate++;
            }
            break;
          case 1:    // Awauting Sync Char 2
            if (tempChar == UBX_SYNC_CHAR2) // 62
            {
              UBXstate++;
            }
            else
            {
              UBXstate = 0; // Wrong sequence so start again
            }
            break;
          case 2: // Awaiting Class
            UBXclass = tempChar;
            UBXchecksum(UBXclass);
            UBXstate++;
            break;
          case 3: // Awaiting Id
            UBXid = tempChar;
            UBXchecksum(UBXid);
            UBXstate++;
            break;
          case 4: // Awaiting Length LSB (little endian so LSB is first)
            UBXlengthLSB = tempChar;
            UBXchecksum(UBXlengthLSB);
            UBXstate++;       
            break;
          case 5:    // Awaiting Length MSB
            UBXlengthMSB = tempChar;
            UBXchecksum(UBXlengthMSB);
            UBXstate++;
            UBXpayloadIdx = 0;
            UBXlength = (byte)(UBXlengthMSB << 8) | UBXlengthLSB; // Convert little endian MSB & LSB into integer
            if (UBXlength >= UBX_MAX_PAYLOAD)
            {
              Serial.println("UBX payload length too large (>100");
              UBXstate=0; // Bad data received so reset and 
              Checksum_A=0;
              Checksum_B=0;
              return false;
            }
            break;
          case 6:
            UBXbuffer[UBXpayloadIdx] = tempChar;
            UBXchecksum(tempChar);
            UBXpayloadIdx++;
            if (UBXpayloadIdx == UBXlength)
            {
              UBXstate++;  // Just processed last byte of payload, so move on
            }
            break;
          case 7:    // Awaiting Checksum 1
            UBXckA = tempChar;
            UBXstate++;
            break;
          case 8:    // Awaiting Checksum 2
            UBXckB = tempChar;
            if ((Checksum_A == UBXckA) && (Checksum_B == UBXckB))    // Check the calculated checksums match actual checksums
            {
              // Checksum is good so parse the message
              parseUBX();
              EOM = true;
            }
            else
            {
              Serial.println("UBX PAYLOAD BAD CHECKSUM!!");
              return false;
            }
            
            UBXstate=0;    // Start again at 0
            Checksum_A=0;
            Checksum_B=0;
            break;
        }
      }
    }
  }
  // We're at the end, which must mean everything's ok.
  return true;
}

void parseUBX() // Chops out only the data I want from the UBX sentence
{
  if(UBXclass == 0x01 && UBXid == 0x07)
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

void UBXchecksum(unsigned char data)
{
  Checksum_A += data;
  Checksum_B += Checksum_A;
}

long join4Bytes(unsigned char data[]) // Joins 4 bytes into one long. Pointers!
{
  union long_union 
  {
    int32_t dword;
    uint8_t  byte[4];
  } longUnion;

  longUnion.byte[0] = *data;
  longUnion.byte[1] = *(data+1);
  longUnion.byte[2] = *(data+2);
  longUnion.byte[3] = *(data+3);
  return(longUnion.dword);
}

// Send a byte array of UBX protocol to the GPS
boolean sendUBX(uint8_t *MSG, uint8_t len)
{
  boolean result;
  Wire.beginTransmission(GPS_ADDR); // Start I2C transmission
  
  //Write one byte at a time
  for(int i=0; i<len; i++)
  {
    if(i > 0 && i % BUFFER_LENGTH == 0) // Wire library bug. Detect 32 bytes (defined in Wire.h) and start again
    {
      if(Wire.endTransmission() != 0)
      {
        Serial.println("Error in Wire.endTransmission at buffer=max.");
        return result;
      }
      
      Wire.beginTransmission(GPS_ADDR);
    }
    
    Serial.print(MSG[i], HEX);
    if(Wire.write(MSG[i]) != 1)
    {
      Serial.println("Error in Wire.write.");
      return result;
    }
  }
  
  if(Wire.endTransmission() != 0)
  {
    Serial.println("Error in Wire.endTransmission.");
    return result;
  }
  
  Serial.println("Success!");
  return true; // Reached the end, no problems encountered
}

// Calculate expected UBX ACK packet and parse UBX response from GPS
boolean getUBX_ACK(uint8_t *MSG) {
  uint8_t readByte;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
  Serial.println("Reading ACK response: ");
 
  // Construct the expected ACK packet    
  ackPacket[0] = 0xB5;	// header
  ackPacket[1] = 0x62;	// header
  ackPacket[2] = 0x05;	// class
  ackPacket[3] = 0x01;	// id
  ackPacket[4] = 0x02;	// length1
  ackPacket[5] = 0x00;  // length2
  ackPacket[6] = MSG[2];	// ACK class
  ackPacket[7] = MSG[3];	// ACK id
  ackPacket[8] = 0;	// CK_A
  ackPacket[9] = 0;	// CK_B
 
  // Calculate the first part of the checksum
  for (uint8_t i=2; i<8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }
 
  while(1)
  {
    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) { 
      Serial.println(" (FAILED!)");
      return false;
    }
    
    // Make sure data is available to read
    Wire.requestFrom(GPS_ADDR, BUFFER_LENGTH); //Request 32 bytes from GPS
    
    while(Wire.available())
    {
      readByte = Wire.read();
      
      if(readByte != 0xFF) //i.e. that there is actually something to read
      {
        // Check that bytes arrive in sequence as per expected ACK packet
        if (readByte == ackPacket[ackByteID])
        { 
          ackByteID++;
          Serial.print(readByte, HEX);
          
          // Test for success
          if (ackByteID > 9)
          {
            // All packets in order!
            Serial.println(" (SUCCESS!)");
            return true;
          }
        }
        else
        {
          ackByteID = 0;	// Reset and look again, invalid order
        }
      } 
    }
  }
}
