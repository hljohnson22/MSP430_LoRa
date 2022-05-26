
#include <SPI.h>
#include <RH_RF95.h>
#include <WiFiNINA_Generic.h>
#include <ArduinoJson.h>
#include <string>

#include "defines.h"
#include "arduino_secrets.h"

int SEESAW_NUM = 3;

// To eliminate FW warning when using not latest nina-fw version
// To use whenever WiFi101-FirmwareUpdater-Plugin is not sync'ed with nina-fw version
#define WIFI_FIRMWARE_LATEST_VERSION        "1.4.5"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP), length must be 8+

int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
char server[] = "httpbin.org";    // name address for server (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

// Singleton instance of the radio driver
RH_RF95 rf95;

// single JSON instance we can write to over and over again
StaticJsonDocument<100> doc; // allocate on stack - we need about 100 bytes to have this working
StaticJsonDocument<100> doc_1; // allocate on stack - we need about 100 bytes to have this working

static int32_t twosComplement(int32_t val, uint8_t bits) {
  if (val & ((uint32_t)1 << (bits - 1))) {
    val -= (uint32_t)1 << bits;
  }
  return val;
}

uint16_t check_crc(uint8_t buffer[], uint16_t bufferLength, uint8_t crcType){
    uint8_t i;
    uint16_t crc;
    uint16_t polynomial;

    polynomial = (crcType == 1) ? 0xC002 : 0x8810; //0x8005 : 0x1021;
    crc = (crcType == 1) ? 0xFFFF : 0x1D0F;

    for(i = 0; i < bufferLength; i++){
      crc = ComputeCrc(crc, buffer[i], polynomial);
    }
    if(crcType == 1){
      return crc;
    }
    else{
      return (uint16_t)(~crc);
    }
}

void printWiFiStatus() 
{
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("Signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

void setup() 
{
  // initialize RF95
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");  
  else
    Serial.println("initialization successful");

  // Initialize ESP32   ???
  Serial.begin(115200);
  while (!Serial);

  Serial.print(F("\nStart WiFiWebClient on ")); Serial.println(BOARD_NAME);
  Serial.println(WIFININA_GENERIC_VERSION);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println(F("Communication with WiFi module failed!"));
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.print(F("Your current firmware NINA FW v"));
    Serial.println(fv);
    Serial.print(F("Please upgrade the firmware to NINA FW v"));
    Serial.println(WIFI_FIRMWARE_LATEST_VERSION);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED)
  {
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println(F("Connected to WiFi"));
  printWiFiStatus();
}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {

      /* MSP430 calaculates by
        smsbTemp = temp[1] << 4;
        lsbTemp = temp[0] >> 4;
        raw = smsbTemp | lsbTemp;
        decimalPlaceValue = (lsbTemp << 4) ^ temp[0];
        finalTemp[0] = raw;
        finalTemp[1] = decimalPlaceValue;
       */
        
       /*
        * CRC calcualted:
        uint16_t CrC = add_crc(finalTemp, 2, 1);
        finalTemp[2] = (CrC & 0xFF00) >> 8;
        finalTemp[3] = (CrC & 0x00FF);
        */
        
        /*
         * 4 bytes of data, 0 and 1 are temp, 2 and 3 is CRC
         */
      if (len == 4) { //check length of packet sent from MSP430
        uint8_t temp[2];
        temp[0] = buf[0];
        temp[1] = buf[1];
        //concat into 1 value

        uint16_t finalTemp = temp[0] << 8 //check shift
        finalTemp = finalTemp || temp[1] //check this is correct val
        
        //compute new crc
        uint16_t newCRC = check_crc(finalTemp, 2, 1)

        //concat old crc
        //concat buf[2] and buf[3] to find CRC that was calculated
        uint16_t oldCRC = buf[2] << 8
        oldCRC = oldCRC || buf[3] //check this is expected. same concat process as temp.


        if(oldCRC == newCRC){
          //   check MSB for temp[0] 1=signed, 0=unsigned
          //   decide temp value, and concat with temp[1] (which is the dedcimala place)

          if(finalTemp || )
          //serialize temp to json string
          String minif_json;
          serializeJson(doc, minif_json); // serialize object to String
          Serial.println(minif_json);
        }
        
        //else drop packet. i.e. do nothing.
      } 
      //send recieved data over wifi
      String content_length_str = "Content-Length: ";
      content_length_str += minif_json.length();

      if (client.connect(server, 80))
      {
        Serial.println(F("Connected to server"));
        // Make a HTTP request:
        client.println("POST /post HTTP/1.1");
        client.println("Host: httpbin.org");
        client.println("Accept: application/json");
        client.println("Content-Type: application/json");
        client.println(content_length_str);
        client.println(minif_json);
        client.println("Connection: close");
        client.println();

        //check wireshark to see if packets are being sent
        //should we implement a firebase version?
        
      }
      Serial.println("Done.");

      // read response body
      while (client.available())
      {
        char c = client.read();
        Serial.write(c);
      }
    
      // if the server's disconnected, stop the client:
      if (!client.connected())
      {
        Serial.println(F("\nDisconnecting from server."));
        client.stop();
      }
    }
    
    else
    {
      Serial.println("recv failed");
    }
  }
}
