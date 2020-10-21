/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#define USE_SERIAL Serial
#include <ArduinoJson.h>

WiFiMulti wifiMulti;


void setup() {

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

}

void loop() {

float temp = 23.;
float humid = 66.;
  
DynamicJsonDocument doc(2048);
doc["deviceId"] = "sensor44";

JsonObject fields = doc.createNestedObject("fields");
fields["temp"] = temp;
fields["humid"] = humid;

String json;
serializeJson(doc, json);
serializeJson(doc, Serial);

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

    delay(5000);
}
