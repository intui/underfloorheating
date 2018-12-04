#ifndef HEADER_INTUIAZURE
  #define HEADER_INTUIAZURE
   
  //Prototype for helper_function found in HelperFunctions.cpp
  String registerSensorAzure(const char* intuiSmartHomeFunction, char* sensorId, char* sensorSecurePin);
  String getProgramFromAzure(const char* intuiSmartHomeFunction, char* sensorId);
  void sendEventToAzure(const char* intuiSmartHomeFunction, String sensorId, String event);
  void sendDataToAzure(const char* intuiSmartHomeFunction, String temperature, String humidity, String sensorId);
#endif
