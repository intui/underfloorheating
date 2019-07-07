#include "Arduino.h"
//#include "intuienvironmentSettings.h"

//#define relaisPin 0

float schmittTriggerDelta = 0.2;
bool schmittTriggerUp = true;
bool relayOn = false;
int relaisPin = D0; //16; //5

// returns 0 for unchanged, 1 for on, 2 for off 
int schmittTriggerSwitch(float t, float targetTemp)
{
  //Serial.println("schmittTriggerSwitch entered. TriggerUp = " + String(schmittTriggerUp)) ;
  
  if (schmittTriggerUp) 
  {
    //Serial.println("t, target - sTD: " + String(t) + ", " + String(targetTemp - schmittTriggerDelta) );
    if(t < targetTemp - schmittTriggerDelta)
    {
      //switch off
     schmittTriggerUp = false;
     digitalWrite(relaisPin, HIGH);
     Serial.println("\n###########\nswitched relais ON\n###########\n");
     relayOn = true;
     return 1;
    }
  }
  else
  {
    //Serial.println("t, target: " + String(t) + ", " + String(targetTemp) );
    if (t > targetTemp)
    {
      // switch on
      schmittTriggerUp = true;
      digitalWrite(relaisPin, LOW);
      Serial.println("\n###########\nswitched relais OFF\n###########\n");
      relayOn = false;
      return 2;
    }
  }
  return 0;

  /*
  Serial.println("\n schmittTriggerSwitch about to exit.");
  Serial.println("lastSwitchOn: " + String(lastSwitchOn));
  Serial.println("lastSwitchOff: " + String(lastSwitchOff));
  Serial.println("morningSwitchOff: " + String(morningSwitchOff));
  Serial.println("millisRunning: " + String(millisRunning));

  Serial.println("enableBoostMode: " + String(enableBoostMode));
  Serial.println("relayBoostOn: " + String(relayBoostOn));
  Serial.println("morningSwitchOff + delayToOn: " + String(morningSwitchOff + delayToOn));
  Serial.println("morningSwitchOff + delayToOff: " + String(morningSwitchOff + delayToOff));
  if(enableBoostMode && millis() > morningSwitchOff + delayToOn && millis() < morningSwitchOff + delayToOff && !relayBoostOn)
  {
    // switch on
     digitalWrite(relaisPin, LOW);
     Serial.println("\n###########\nswitched relais BOOST ON\n###########\n");
     relayBoostOn = true;
     sendDataToAzure(String(t), "-2.1");
  }
  if(enableBoostMode && millis() > morningSwitchOff + delayToOff && relayBoostOn)
  {
    // switch off
      digitalWrite(relaisPin, HIGH);
      Serial.println("\n###########\nswitched relais BOOST OFF\n###########\n");
      relayBoostOn = false;
      sendDataToAzure(String(t), "-2.2");
  }
  */
}
