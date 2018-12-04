#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Time.h>
#include <TimeLib.h>
#include <ArduinoJson.h>

//#include "intuienvironmentSettings.h"

WiFiClient client;

String registerSensorAzure(const char* intuiSmartHomeFunction, char* sensorId, char* sensorSecurePin)
{
  String returnVal = "";
  Serial.println("registerSensorAzure entered.");
  if( client.connect(intuiSmartHomeFunction, 80))
  {
    Serial.write("connected to: ");
    Serial.println(intuiSmartHomeFunction);
    // POST URI
    client.print("GET /api/Register"); //client.print(table_name); 
    client.print("?id=");
    client.print(sensorId);
    client.print("&pin=");
    client.print(sensorSecurePin);
    client.println(" HTTP/1.1");
    // Host header
    client.print("Host:"); client.println(intuiSmartHomeFunction);
    // Azure Mobile Services application key
    client.print("X-ZUMO-APPLICATION:"); client.println("intuiSmarthome");
    // JSON content type
    client.println("Content-Type: application/json");
    client.print("Content-Length:"); client.println(0);//"102");
    // End of headers
    client.println(); 

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return("");
    }
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return("");
    }
    client.find("\n");
    returnVal = client.readStringUntil('\r');
    Serial.println("registerSensorAzure returnVal: " + returnVal);
    /*
    while (client.available()) 
      {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      returnVal = line;
      }
    client.stop();
    */
    // returns servertime in seconds since 1970
    return returnVal;
  }
}

String getProgramFromAzure(const char* intuiSmartHomeFunction, char* sensorId)
{
  String returnVal = "";
  Serial.println("getProgramFromAzureAzure entered.");
  if( client.connect(intuiSmartHomeFunction, 80))
  {
    Serial.write("connected to: ");
    Serial.println(intuiSmartHomeFunction);
    // POST URI
    client.print("GET /api/Program"); //client.print(table_name); 
    client.print("?id=");
    client.print(sensorId);
    client.println(" HTTP/1.1");
    // Host header
    client.print("Host:"); client.println(intuiSmartHomeFunction);
    // Azure Mobile Services application key
    client.print("X-ZUMO-APPLICATION:"); client.println("intuiSmarthome");
    // JSON content type
    //client.println("Content-Type: application/json");
    //client.print("Content-Length:"); client.println(0);//"102");
    // End of headers
    client.println();    
    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return("");
    }
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return("");
    }
    client.find("\n");

    // Allocate JsonBuffer
    // Use arduinojson.org/assistant to compute the capacity.
    //const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3) + 126;
    const size_t capacity = 400;
    DynamicJsonBuffer jsonBuffer(capacity);
  
    Serial.println("capacity: ");
    Serial.println(String(capacity));
    // Parse JSON object
    //{"SensorId":"000-00000-007","programItems":[{"ProgramItemId":0,"Seconds":28800,"TargetValue":23.0},{"ProgramItemId":1,"Seconds":68400,"TargetValue":18.0}]}

    String retVal = client.readStringUntil('\r');
    return retVal;
    // del:
    JsonObject& root = jsonBuffer.parseObject(client);
    if (!root.success()) {
      Serial.println(F("Parsing failed!"));
      return("");
    }
    float targetTemp = root["programItems"][0]["TargetValue"];
    returnVal = targetTemp; //atof(targetTemp);
    Serial.println("targetTemp: ");
    Serial.println(targetTemp);
  
    client.stop();
    Serial.println();
    Serial.println("returnVal: ");
    Serial.println(returnVal);
    // returns program as json
    return returnVal;
  }
}

void sendEventToAzure(const char* intuiSmartHomeFunction, String sensorId, String event)
{
  Serial.println("sendEventToAzure entered.");
  String captureHour = String(hour());
  String captureMinute = String(minute());
  String captureSecond = String(second());

  if(captureHour.length() == 1)
  {
    captureHour = "0" + captureHour;
  }
  if(captureMinute.length() == 1)
  {
    captureMinute = "0" + captureMinute;
  }
  if(captureSecond.length() == 1)
  {
    captureSecond = "0" + captureSecond;
  }

  String captureTime = String(year()) + "-" + String(month()) + "-" + String(day()) + "T" + captureHour + ":" + captureMinute + ":" + captureSecond + ".000";
  String jsonStr = "{\"SensorId\":\"" + sensorId +"\", \"EventTime\": \"" + captureTime + "\", \"Event\":\"" + event + "\"}";
  if( client.connect(intuiSmartHomeFunction, 80))
  {
    Serial.write("connected to: ");
    Serial.println(intuiSmartHomeFunction);
    Serial.println(jsonStr);

    int millisBefore = millis();  

    // POST URI
    client.print("POST /api/PostEvent/"); //client.print(table_name); 
    client.println(" HTTP/1.1");
    // Host header
    client.print("Host:"); client.println(intuiSmartHomeFunction);
    // Azure Mobile Services application key
    client.print("X-ZUMO-APPLICATION:"); client.println("intuiSmarthome");
    // JSON content type
    client.println("Content-Type: application/json");
    // Content length
    client.print("Content-Length:"); client.println(jsonStr.length());
    // End of headers
    client.println();
    // POST message body
    client.println(jsonStr);
    
    while (client.available()) 
      {
      char c = client.read();
      Serial.write(c);
      }
    Serial.println();
    client.stop();
    int duration = millis() - millisBefore;
    Serial.print("Duration: ");
    Serial.println(String(duration));
    Serial.println();
  }
}
void sendDataToAzure(const char* intuiSmartHomeFunction, String temperature, String humidity, String sensorId)
{
  Serial.println("sendDataToAzure entered.");
  String captureHour = String(hour());
  String captureMinute = String(minute());
  String captureSecond = String(second());

  if(captureHour.length() == 1)
  {
    captureHour = "0" + captureHour;
  }
  if(captureMinute.length() == 1)
  {
    captureMinute = "0" + captureMinute;
  }
  if(captureSecond.length() == 1)
  {
    captureSecond = "0" + captureSecond;
  }

  String captureTime = String(year()) + "-" + String(month()) + "-" + String(day()) + "T" + captureHour + ":" + captureMinute + ":" + captureSecond + ".000";
  String jsonStr = "{\"AmbienceId\": 0, \"captureTime\": \"" + captureTime + "\", \"temperature\":" + temperature + ", \"humidity\":" + humidity + ", \"SensorId\":\"" + sensorId +"\"}";
  if( client.connect(intuiSmartHomeFunction, 80))
  {
    Serial.write("connected to: ");
    Serial.println(intuiSmartHomeFunction);
    Serial.println(jsonStr);

    int millisBefore = millis();  

    // POST URI
    client.print("POST /api/PostAmbience/"); //client.print(table_name); 
    client.println(" HTTP/1.1");
    // Host header
    client.print("Host:"); client.println(intuiSmartHomeFunction);
    // Azure Mobile Services application key
    client.print("X-ZUMO-APPLICATION:"); client.println("intuiSmarthome");
    // JSON content type
    client.println("Content-Type: application/json");
    // Content length
    client.print("Content-Length:"); client.println(jsonStr.length());
    // End of headers
    client.println();
    // POST message body
    client.println(jsonStr);
    
    while (client.available()) 
      {
      char c = client.read();
      Serial.write(c);
      }
    Serial.println();
    client.stop();
    int duration = millis() - millisBefore;
    Serial.print("Duration: ");
    Serial.println(String(duration));
    Serial.println();
  }
}
