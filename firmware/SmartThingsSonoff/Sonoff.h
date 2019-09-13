
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
    Relay* relay1;
    Relay* relay2;
    Relay* relay3;
    Relay* relay4;
    Switch* sw1;
    Switch* sw2;
    Switch* sw3;
    Switch* sw4;

//    Relay* getRelay(int ch) {
//      switch (ch) {
//        case 1: {
//            return this->relay1;
//          }
//        case 2: {
//            return this->relay2;
//          }
//        case 3: {
//            return this->relay3;
//          }
//        case 4: {
//            return this->relay4;
//          }
//        default: {
//            return this->relay1;
//          }
//      }
//
//    }
//
//    Switch* getSwitch(int ch) {
//      switch (ch) {
//        case 1: {
//            return this->sw1;
//          }
//        case 2: {
//            return this->sw2;
//          }
//        case 3: {
//            return this->sw3;
//          }
//        case 4: {
//            return this->sw4;
//          }
//        default: {
//            return this->sw1;
//          }
//      }
//    }

  public:

    Sonoff(Storage* storage ) {
      this->storage = storage;
      if (storage->getRelayPin(1) > -1) {
        this->relay1 = new Relay( storage->getRelayPin(1), 0.4, 500, 25000);
      }
      if (storage->getSwitchPin(1) > -1) {
        this->sw1 =  new Switch( storage->getSwitchPin(1), 2, 5);
      }

      if (storage->getRelayPin(2) > -1) {
        this->relay2 = new Relay( storage->getRelayPin(2), 0.4, 500, 25000);
      }
      if (storage->getSwitchPin(2) > -1) {
        this->sw2 =  new Switch( storage->getSwitchPin(2), 2, 5);
      }

      if (storage->getRelayPin(3) > -1) {
        this->relay3 = new Relay( storage->getRelayPin(3), 0.4, 500, 25000);
      }
      if (storage->getSwitchPin(3) > -1) {
        this->sw3 =  new Switch( storage->getSwitchPin(3), 2, 5);
      }

      if (storage->getRelayPin(4) > -1) {
        this->relay4 = new Relay( storage->getRelayPin(4), 0.4, 500, 25000);
      }
      if (storage->getSwitchPin(4) > -1) {
        this->sw4 =  new Switch( storage->getSwitchPin(4), 2, 5);
      }
    }

    void loop() {
      if (storage->getRelayPin(1) > -1) {
        relay1->loop();
      }
      if (storage->getSwitchPin(1) > -1) {
        sw1->loop();
      }

      if (storage->getRelayPin(2) > -1) {
        relay2->loop();
      }
      if (storage->getSwitchPin(2) > -1) {
        sw2->loop();
      }

      if (storage->getRelayPin(3) > -1) {
        relay3->loop();
      }
      if (storage->getSwitchPin(3) > -1) {
        sw3->loop();
      }

      if (storage->getRelayPin(4) > -1) {
        relay4->loop();
      }
      if (storage->getSwitchPin(4) > -1) {
        sw4->loop();
      }
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

    int getRelayStatus(int ch) {
      int relayStatus = -1;
      switch (ch) {
        case 1: {
            if (storage->getRelayPin(1) > -1) {
              relayStatus = relay1->isOn();
            }
          }
        case 2: {
            if (storage->getRelayPin(2) > -1) {
              relayStatus = relay2->isOn();
            }
          }
        case 3: {
            if (storage->getRelayPin(3) > -1) {
              relayStatus = relay3->isOn();
            }
          }
        case 4: {
            if (storage->getRelayPin(4) > -1) {
              relayStatus = relay4->isOn();
            }
          }
        default: {
            if (storage->getRelayPin(1) > -1) {
              relayStatus = relay1->isOn();
            }
          }
      }
    }

    void relayOn(int ch) {
      switch (ch) {
        case 1: {
            if (storage->getRelayPin(1) > -1) {
              relay1->on();
            }
          }
        case 2: {
            if (storage->getRelayPin(2) > -1) {
              relay2->on();
            }
          }
        case 3: {
            if (storage->getRelayPin(3) > -1) {
              relay3->on();
            }
          }
        case 4: {
            if (storage->getRelayPin(4) > -1) {
              relay4->on();
            }
          }
        default: {
            if (storage->getRelayPin(1) > -1) {
              relay1->on();
            }
          }
      }
    }

    void relayOff(int ch) {
      switch (ch) {
        case 1: {
            if (storage->getRelayPin(1) > -1) {
              relay1->off();
            }
          }
        case 2: {
            if (storage->getRelayPin(2) > -1) {
              relay2->off();
            }
          }
        case 3: {
            if (storage->getRelayPin(3) > -1) {
              relay3->off();
            }
          }
        case 4: {
            if (storage->getRelayPin(4) > -1) {
              relay4->off();
            }
          }
        default: {
            if (storage->getRelayPin(1) > -1) {
              relay1->off();
            }
          }
      }
    }

    String getRelayStatusAsString(int ch) {
      switch (getRelayStatus(ch)) {
        case 0: {
            return "off";
          }
        case 1: {
            return "on";
          }
        default: {
            return "undefined";
          }
      }
    }

    int getSwitchEvent(int ch) {
      int ret = -1;
      switch (ch) {
        case 1: {
            if (storage->getSwitchPin(1) > -1) {
              ret = sw1->getEvent();
            }
          }
        case 2: {
            if (storage->getSwitchPin(2) > -1) {
              ret = sw2->getEvent();
            }
          }
        case 3: {
            if (storage->getSwitchPin(3) > -1) {
              ret = sw3->getEvent();
            }
          }
        case 4: {
            if (storage->getSwitchPin(4) > -1) {
              ret = sw4->getEvent();
            }
          }
        default: {
            if (storage->getSwitchPin(1) > -1) {
              ret = sw1->getEvent();
            }
          }
      }
      return ret;
    }



    uint8_t getLed() {
      return this->storage->getLedPin();
    }
};
#endif /* Sonoff_h */
