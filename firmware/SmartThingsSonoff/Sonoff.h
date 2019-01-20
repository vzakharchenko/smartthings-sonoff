
#ifndef Sonoff_h
#define Sonoff_h

#include "relay.h"
#include "switch.h"

#define ARDUINO_ESP8266_ESP01

#ifdef ARDUINO_ESP8266_ESP01           // Generic ESP's
#define D0 16
#define D1 5
#define D2
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#endif

#define SONOFF_BASIC 0
#define SONOFF_INTERCOM 1
#define SONOFF_BASIC_GPIO14 2
#define SONOFF_POW 3

class Sonoff
{

  private:
    Storage* storage;
    boolean remoteButtonState = false;
    boolean remoteButtonStateLast = false;
    unsigned long remoteButtonStateTime = millis();
    Relay* relay;
    Switch* sw;

    //static const uint8_t RELAY = RELAYPIN; // MTDI
    //static const uint8_t SWITCH = D3; // GPIO0
    //static const uint8_t LED = LEDgreen; // MTCK
  public:

    Sonoff(Storage* storage ) {
      this->storage = storage;
      this->relay = new Relay( D6, 0.4, 500, 25000);
      this->sw =  new Switch( D3, 2, 5);
    }

    void loop() {
      relay->loop();
      sw->loop();
    }

    void setup() {
      if (this->storage->getDeviceType() == SONOFF_BASIC_GPIO14 || this->storage->getDeviceType() == SONOFF_INTERCOM ) {
        pinMode(D5, INPUT);
      }
    }


    boolean IsButtonChanged() {
      boolean buttonState = IsButtonOn();
      if (remoteButtonStateLast != buttonState ) {
        if (this->storage->getDeviceType() == SONOFF_INTERCOM) {
          unsigned long remoteButtonCurrentStateTime = millis();
          if ((remoteButtonCurrentStateTime - remoteButtonStateTime) > (storage->getIntercomCallTimeout())) {
            remoteButtonStateLast =  buttonState;
            remoteButtonStateTime = millis();
            return true;
          } else {
             remoteButtonStateTime = millis(); 
            return false;
          }
        } else {
          remoteButtonStateLast =  buttonState;
          remoteButtonStateTime = millis();
          return true;
        }
      }
      return false;
    }

    boolean IsButtonOn() {
      if (this->storage->getDeviceType() == SONOFF_BASIC_GPIO14 || this->storage->getDeviceType() == SONOFF_INTERCOM ) {
        remoteButtonState = digitalRead(D5);
        return remoteButtonState == this->storage-> getGpio14State();
      } else {
        return false;
      }
    }

    Relay* getRelay() {
      return this->relay;
    }

    Switch* getSwitch() {
      return this->sw;
    }

    uint8_t getLed() {
      if (this->storage->getDeviceType() == SONOFF_POW) {
        return D8;
      } else {
        return D7;
      }
    }
};
#endif /* Sonoff_h */
