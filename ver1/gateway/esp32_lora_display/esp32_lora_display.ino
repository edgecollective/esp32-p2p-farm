/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */
#include <U8x8lib.h>
#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#define USE_SERIAL Serial
#include <ArduinoJson.h>

WiFiMulti wifiMulti;

#define RFM95_CS 18
#define RFM95_RST 14
#define RFM95_INT 26


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

String receivedText;
String receivedRssi;

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);


unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 90 seconds (5000)
unsigned long timerDelay = 90000;

StaticJsonDocument<200> doc;


void setup() {

u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  u8x8.drawString(0, 0, "Gateway on!");
  
    USE_SERIAL.begin(9600);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    wifiMulti.addAP("InmanSquareOasis", "portauprince");

while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);



}

void loop() {

if (rf95.available())
  {

    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      u8x8.drawString(0, 4, (char*)buf);

      DeserializationError error = deserializeJson(doc, (char*)buf);

if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }

      Serial.println("Parsed JSON:");
      serializeJsonPretty(doc, Serial);
      Serial.println();

      Serial.print("SIZE:");
      int mysize = measureJson(doc);
      Serial.println(mysize);

      // put in json structure for upload
      DynamicJsonDocument updoc(2048);
updoc["deviceId"] = "sensor44";

JsonObject fields = updoc.createNestedObject("fields");
fields["temp"] = doc["tempC_19"];
fields["CO2"] = doc["ppm_19"];

String json;
serializeJson(updoc, json);
serializeJson(updoc, Serial);


      // wait for WiFi connection
    if((wifiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
        //http.begin("http://example.com/index.html"); //HTTP
        http.begin("http://64.227.0.108:3000/api/drives/dca4331a61ba5f729d1bca586023f65e5e2590b48519cd36aad7be013e0fe9c8");
        
        http.addHeader("Authorization","92dc55abe6615f3741680ef25863ffae80d2f8924a0c0660b6a46a0c413fe21c");
        
        http.addHeader("Content-Type", "application/json");
        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        
      
        int httpCode = http.PUT(json);        

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }

    delay(60000); // wait a minute until the next post
      
    }
} // end if (rf95.available())

} // end loop
