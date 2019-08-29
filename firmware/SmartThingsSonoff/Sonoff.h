
#ifndef Sonoff_h
#define Sonoff_h

#include "relay.h"
#include "switch.h"

#define ARDUINO_ESP8266_ESP01

#ifdef ARDUINO_ESP8266_ESP01           // Generic ESP's
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#endif

class Sonoff
{

  private:
    Storage* storage;
    boolean remoteButtonState = false;
    boolean remoteButtonStateLast = false;
    unsigned long remoteButtonStateTime = millis();
    Relay* relay;
    Switch* sw;
  public:

    Sonoff(Storage* storage ) {
      this->storage = storage;
      this->relay = new Relay( storage->getRelayPin(), 0.4, 500, 25000);
      this->sw =  new Switch( storage->getSwitchPin(), 2, 5);
    }

    void loop() {
      relay->loop();
      sw->loop();
    }

    void setup() {
      if (this->storage->getExternalSwitchPin() > -1) {
        pinMode(this->storage->getExternalSwitchPin(), INPUT);
      }
    }


    boolean IsButtonChanged() {
      boolean buttonState = IsButtonOn();
      if (remoteButtonStateLast != buttonState ) {
        remoteButtonStateLast =  buttonState;
        remoteButtonStateTime = millis();
        return true;
      }
      return false;
    }

    boolean IsButtonOn() {
      if (this->storage->getExternalSwitchPin() > -1 ) {
        remoteButtonState = digitalRead(this->storage->getExternalSwitchPin());
        return remoteButtonState == this->storage-> getExternalSwitchState();
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
      return this->storage->getLedPin();
    }
};
#endif /* Sonoff_h */
