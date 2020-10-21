/*
  Basic Arduino example for K-Series sensor
  Created by Jason Berger
  Co2meter.com  
*/


#include <SPI.h>
#include <Wire.h>
#include <RH_RF95.h>
#include <ArduinoJson.h>
#include "MHZ19.h"    

#include "SoftwareSerial.h"

#define RX_PIN 3                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 4                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)

MHZ19 myMHZ19;                                             // Constructor for library

SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial
//HardwareSerial mySerial(1);                              // (ESP32 Example) create device to MH-Z19 serial

unsigned long getDataTimer = 0;

int millisWait = 60000; // 5 sec

int packetlength=80;

// for mothbot RMF95
#define RFM95_CS 8
#define RFM95_RST 7
#define RFM95_INT 2

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

    StaticJsonDocument<200> doc;

int mothled = 5;

void setup() 
{

pinMode(mothled, OUTPUT);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // put your setup code here, to run once:
  Serial.begin(9600);         //Opens the main serial port to communicate with the computer

   mySerial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start   
    //mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN); // (ESP32 Example) device to MH-Z19 serial start   
    myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin(). 

    myMHZ19.autoCalibration(false);                              // Turn auto calibration ON (OFF autoCalibration(false))
    
  
  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

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
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  
}

void loop() 
{

  if (millis() - getDataTimer >= millisWait)
    {
        int CO2; 

        /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even 
        if below background CO2 levels or above range (useful to validate sensor). You can use the 
        usual documented command with getCO2(false) */

        CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
        //CO2 = 203;
        
        Serial.print("CO2 (ppm): ");                      
        Serial.println(CO2);                                

        int8_t Temp;
        Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
        
        //Temp = 22;
        Serial.print("Temperature (C): ");                  
        Serial.println(Temp);                               

  //doc["sensorID"]=19;
  doc["ppm_19"]=CO2;
  doc["tempC_19"]=Temp;

char radiopacket[70];
  serializeJson(doc, radiopacket);
  
  Serial.print("Sending "); Serial.println(radiopacket);
  //radiopacket[19] = 0;
  
  Serial.println("Sending...");
  delay(10);
  rf95.send((uint8_t *)radiopacket, 70);

  Serial.println("Waiting for packet to complete..."); 
  delay(10);
  rf95.waitPacketSent();
Serial.println("packet sent.");
  digitalWrite(mothled, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(mothled, LOW);    // turn the LED off by making the voltage LOW
  delay(100);                       // wait for a second

     getDataTimer = millis();
    }
}
