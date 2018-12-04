/*
 * bme280_example.ino
 * Example sketch for bme280
 *
 * Copyright (c) 2016 seeed technology inc.
 * Website    : www.seeedstudio.com
 * Author     : Lambor
 * Create Time:
 * Change Log :
 *
 * The MIT License (MIT)

  "Hello World" version for U8x8 API

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
*/

#include "Seeed_BME280.h"
#include <Wire.h>
#include <U8x8lib.h>
#include <ESP8266WiFi.h>
#include <Time.h>
#include <TimeLib.h>
#include <ArduinoJson.h>

#include "intuienvironmentAzure.h"
#include "intuienvironmentSettings.h"
#include "schmittTriggerSwitch.h"

#define sensorPin A0

float minTemp = 15.0;
float maxTemp = 40.0;

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
BME280 bme280;

//WiFiClient client;

int lastupload = 0;
int lastProgPull = 0;
int lastSchmittTrig = 0;
int timeZone = 1;
bool WiFiConnected = false;
float targetTemp = 0.0f;

void setup()
{
  Serial.begin(115200);
  Serial.println("setup started.");
  pinMode(16, OUTPUT); // Wemos D1 = 2

  if(!bme280.init()){
    Serial.println("Device error!");
  }
  initializeDisplay();
  connectWifi();
 
  Serial.println();
  String bootTime = registerSensorAzure(intuiSmartHomeFunction, sensorId, sensorSecurePin);
  Serial.println("bootTime: " + bootTime);
  int bootMillis = millis();
  setTime(bootTime.toInt());
  adjustTime(timeZone * 3600);
  Serial.print("it is: "); Serial.print(hour()); Serial.print(":"); Serial.print(minute()); Serial.print(".");Serial.println(second());
  //lastupload = 0;
  getProgram();
}

void loop()
{
  float temperatureSelectValue = analogRead(sensorPin);
  targetTemp = minTemp + (maxTemp - minTemp) * (temperatureSelectValue/980.0f);
  printEnv(targetTemp);
  printTime();
  float t = bme280.getTemperature();

  if (millis() - lastSchmittTrig > 2*1000)
  {
    Serial.println("Knopf: " + String(temperatureSelectValue));
    schmittTriggerSwitch(t, targetTemp);
    lastSchmittTrig = millis();
  }
  if (millis() - lastupload > 1*60*1000)
  {
    uploadAmbience();
    lastupload = millis();
  }
  // ToDo: ask API for anyNews to call getProgram in case.
  if (millis() - lastProgPull > 5 * 60 * 1000)
  {
     getProgram();
     lastProgPull = millis();  
  }
  delay(100);
}

void getProgram()
{
  String program = getProgramFromAzure(intuiSmartHomeFunction, sensorId);

  //ToDo:
  targetTemp = parseProgram(program);
  
  // get Program from function
  // deserialize result like that:
  
}

float parseProgram(String programStr)
{
  Serial.println("parseProgram entered.");
 
  char JSONMessage[] = " {\"SensorType\": \"Temperature\", \"Value\": 10}"; //Original message
  Serial.print("Initial string value: ");
  Serial.println(JSONMessage);
 
  StaticJsonBuffer<300> JSONBuffer;   //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(JSONMessage); //Parse message
 
  if (!parsed.success()) {   //Check for errors in parsing
 
    Serial.println("Parsing failed");
    delay(5000);  
 }
  const char * sensorType = parsed["SensorType"]; //Get sensor type value
  int value = parsed["Value"];                                         //Get value of sensor measurement
 
  Serial.println(sensorType);
  Serial.println(value);
 
  return 0.0;
}

void uploadAmbience()
{
  sendDataToAzure(intuiSmartHomeFunction, String(bme280.getTemperature()), String(bme280.getHumidity()), sensorId);
}

void initializeDisplay()
{
  u8x8.begin();
  u8x8.setPowerSave(0);
  //u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setFont(u8x8_font_torussansbold8_r); 
  //u8x8.draw2x2String(0,0,"Temp");
  u8x8.drawString(0,0,"Temp");
  u8x8.drawString(0,5,"Soll");
}

void printEnv(float targetTemp)
{
  //get and print temperatures
  char temperaturStr[6];
  dtostrf(bme280.getTemperature(), 2, 1, temperaturStr);
  char sollTemp[6];
  dtostrf(targetTemp, 2, 1, sollTemp);
  //String msg = String(bme280.getTemperature());
  //u8x8.drawString(0,1,"Hi");
//  u8x8.setCursor(0,1);
  u8x8.draw2x2String(0,2,temperaturStr);
//  u8x8.setCursor(0,6);
  u8x8.draw2x2String(0,6,sollTemp);
}
void printTime()
{
  String hourStr = String(hour());
  if(hourStr.length() == 1)
    hourStr = "0" + hourStr;
  String minuteStr = String(minute());
  if(minuteStr.length() == 1)
    minuteStr = "0" + minuteStr;
  String secondStr = String(second());
  if(secondStr.length() == 1)
    secondStr = "0" + secondStr;

  String timeNow = hourStr + ":" + minuteStr + "." + secondStr;
  u8x8.setCursor(8,0);
  u8x8.print(timeNow);
}

void connectWifi()
{
  WiFi.begin(ssid, password);
  int i;
  for(i=0;i<20;i++)
  {
    if(WiFi.status() == WL_CONNECTED)
      {
        Serial.println("My WiFi connected");
        Serial.println(ssid);
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        delay(20);
        WiFiConnected = true;
        break;
      }
    delay(500);
    Serial.print(",");
  }
  if(!WiFiConnected)
  {
    WiFi.begin(ssid2, password2);
    int i;
    for(i=0;i<20;i++)
    {
      if(WiFi.status() == WL_CONNECTED)
        {
          Serial.println("My WiFi connected");
          Serial.println(ssid2);
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
          delay(20);
          WiFiConnected = true;
          break;
        }
      delay(500);
      Serial.print(",");
    }
  }
  if(!WiFiConnected)
  {
    WiFi.begin(ssid3, password3);
    int i;
    for(i=0;i<20;i++)
    {
      if(WiFi.status() == WL_CONNECTED)
        {
          Serial.println("My WiFi connected");
          Serial.println(ssid3);
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
          delay(20);
          WiFiConnected = true;
          break;
        }
      delay(500);
      Serial.print(",");
    }
  }
}



