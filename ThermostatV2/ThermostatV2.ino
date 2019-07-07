#include "Seeed_BME280.h"
#include <Wire.h>
#include <U8x8lib.h>
#include <ESP8266WiFi.h>
#include <Time.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <Encoder.h>

#include "intuienvironmentAzure.h"
#include "intuienvironmentSettings.h"
#include "schmittTriggerSwitch.h"

//#define sensorPin A0

float minTemp = 15.0;
float maxTemp = 30.0;

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
BME280 bme280;

//WiFiClient client;

int lastupload = 0;
int lastProgPull = 0;
int lastSchmittTrig = 0;
int timeZone = 1;
int bootTimeInt = 0; // Servertime
bool WiFiConnected = false;
float targetTemp = 0.0f;
int manualUpdateTime = 0;
float manualTemperatureValue = 0; //22.5;
int serverUpdateTime = 0;
float serverTemperatureValue = 0;
bool manualTemperatureUpdated = false;

Encoder myEnc(D5, D6);

void setup()
{
  Serial.begin(115200);
  Serial.println("setup started.");
  pinMode(D0, OUTPUT); // Wemos D0 = 16 // move to settings
  Serial.println("initializing MBE280 sensor.");

  //Wire.begin(D2, D1);
  //Serial.println("SDA: " + String(SDA)); // 4
  //Serial.println("SCL: " + String(SCL)); // 5
  if(!bme280.init()){
    Serial.println("Device error!");
  }
  Serial.println("initializing display");
  initializeDisplay();
  
  connectWifi();
 
  Serial.println();
  String bootTime = registerSensorAzure(intuiSmartHomeFunction, sensorId, sensorSecurePin);
  Serial.println("bootTime: " + bootTime);
  bootTimeInt = bootTime.toInt();
  //int bootMillis = millis();
  setTime(bootTimeInt);
  adjustTime(timeZone * 3600);
  Serial.print("it is: "); Serial.print(hour()); Serial.print(":"); Serial.print(minute()); Serial.print(".");Serial.println(second());
  //lastupload = 0;
  //getProgram();
  myEnc.write(300);

  sendEventToAzure(intuiSmartHomeFunction, sensorId, "device started");
}

void loop()
{
  //targetTemp = 
  readTemperatureKnob();
  //long newPosition = myEnc.read();
  printEnv(targetTemp);
  printTime();
  float t = bme280.getTemperature();

  if (millis() - lastSchmittTrig > 0.5*1000)
  {
    //Serial.println("Target: " + String(targetTemp));
    int triggerValue = schmittTriggerSwitch(t, targetTemp);
    if(triggerValue == 1)
    {
      sendEventToAzure(intuiSmartHomeFunction, sensorId, "{\\\"HeatingValue\\\": true }");
    }
    if(triggerValue == 2)
    {
      sendEventToAzure(intuiSmartHomeFunction, sensorId, "{\\\"HeatingValue\\\": false }");
    }    
    lastSchmittTrig = millis();
  }
  if (millis() - lastupload > 1*60*1000)
  {
    uploadAmbience();
    lastupload = millis();
  }
  // ToDo: ask API for anyNews to call getProgram in case.
  if (millis() - lastProgPull > 2 * 60 * 1000)
  {
     getProgram();
     lastProgPull = millis();  
  }

  if(manualTemperatureUpdated && (bootTimeInt + (millis() / 1000)) - manualUpdateTime > 5 )
  {
    sendEventToAzure(intuiSmartHomeFunction, sensorId, "{\\\"targetTemperature\\\": \\\"" + String(targetTemp) + "\\\"}");
    manualTemperatureUpdated = false;
  }
  //delay(25);
}

void readTemperatureKnob()
{
  //float temperatureSelectValue = analogRead(sensorPin);
  float temperatureSelectValue = myEnc.read();
  float tempValue = minTemp + temperatureSelectValue / 40.0f;
  if(abs(manualTemperatureValue - tempValue) > 0.1)
  {
    manualTemperatureValue = tempValue;
    manualUpdateTime = bootTimeInt + (millis() / 1000);
    Serial.print("TargetTemp manually updated to: ");
    Serial.print(manualTemperatureValue);
    Serial.print(" at: ");
    Serial.println(manualUpdateTime);
    if(manualUpdateTime > serverUpdateTime)
    {
      targetTemp = manualTemperatureValue;
      Serial.println("readTemperatureKnob set targetTemp to: " + String(targetTemp));
      manualTemperatureUpdated = true;
    }
  }
  //return manualTemperatureValue;
}

void getProgram()
{
  String program = getProgramFromAzure(intuiSmartHomeFunction, sensorId);
  Serial.println("got program: " + program);
  parseProgram(program);
}

void parseProgram(String programStr)
{
  Serial.println("parseProgram entered.");
 
  StaticJsonBuffer<400> JSONBuffer;   //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(programStr);
 //{"SensorId":"000-00000-007","TimeOfCreation":1542192927,"programItems":[{"ProgramItemId":0,"Seconds":21600,"TargetValue":23.0},{"ProgramItemId":1,"Seconds":68400,"TargetValue":18.0}]}
  if (!parsed.success()) 
  {   
    Serial.println("Parsing failed");
    delay(500);  
    return;    
  }
  Serial.println("parsed program: " + programStr);
  float serverTemperatureValue = parsed["programItems"][0]["TargetValue"];
  int serverUpdateTime = parsed["TimeOfCreation"];

  if(manualUpdateTime <= serverUpdateTime)
  {
    targetTemp = serverTemperatureValue;
    Serial.println("parseProgram set targetTemp to: " + String(targetTemp));
  }
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



