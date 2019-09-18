
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
    }

    void init() {
      if (storage->getRelayPin(1) > -1) {
        Serial.println ( "created relay pin 1 " + String(storage->getRelayPin(1)) );
        this->relay1 = new Relay( storage->getRelayPin(1));
      }
      if (storage->getSwitchPin(1) > -1) {
        Serial.println ( "created switch pin 1 " + String(storage->getSwitchPin(1)) );
        this->sw1 =  new Switch( storage->getSwitchPin(1), false, 2, 5);
      }

      if (storage->getRelayPin(2) > -1) {
                Serial.println ( "created relay pin 2 " + String(storage->getRelayPin(2)) );  
        this->relay2 = new Relay( storage->getRelayPin(2));
        Serial.println ( "created relay pin 3 " + String(storage->getRelayPin(3)) );
      }
      if (storage->getSwitchPin(2) > -1) {
        this->sw2 =  new Switch( storage->getSwitchPin(2), false, 2, 5);
      }

      if (storage->getRelayPin(3) > -1) {
        this->relay3 = new Relay( storage->getRelayPin(3));
      }
      if (storage->getSwitchPin(3) > -1) {
        this->sw3 =  new Switch( storage->getSwitchPin(3), false, 2, 5);
      }

      if (storage->getRelayPin(4) > -1) {
        this->relay4 = new Relay( storage->getRelayPin(4));
      }
      if (storage->getSwitchPin(4) > -1) {
        this->sw4 =  new Switch( storage->getSwitchPin(4), false, 2, 5);
      }
    }

    void loop() {
      if ( this->storage->getRelayPin(1) > -1) {
        // Serial.println ( "relay 1 loop" );
        this->relay1->loop();
      }
      if ( this->storage->getSwitchPin(1) > -1) {
        //Serial.println ( "switch 1 loop" );
        this->sw1->loop();
      }

      if ( this->storage->getRelayPin(2) > -1) {
        this->relay2->loop();
      }
      if ( this->storage->getSwitchPin(2) > -1) {
        this->sw2->loop();
      }

      if ( this->storage->getRelayPin(3) > -1) {
        this->relay3->loop();
      }
      if ( this->storage->getSwitchPin(3) > -1) {
        this->sw3->loop();
      }

      if ( this->storage->getRelayPin(4) > -1) {
        this->relay4->loop();
      }
      if ( this->storage->getSwitchPin(4) > -1) {
        this->sw4->loop();
      }

    }

    void setup() {
      if (this->storage->getExternalSwitchPin() > -1) {
        pinMode(this->storage->getExternalSwitchPin(), INPUT);
      }
      init();
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

    bool getRelayStatus(int ch) {
     bool relayStatus = false;
      switch (ch) {
        case 1: {
            if (storage->getRelayPin(1) > -1) {
              relayStatus = relay1->isOn();
            }
            break;
          }
        case 2: {
            if (storage->getRelayPin(2) > -1) {
              relayStatus = relay2->isOn();
            }
            break;
          }
        case 3: {
            if (storage->getRelayPin(3) > -1) {
              relayStatus = relay3->isOn();
            }
            break;
          }
        case 4: {
            if (storage->getRelayPin(4) > -1) {
              relayStatus = relay4->isOn();
            }
            break;
          }
        default: {
          Serial.println("getRelayStatus undefined channel");
            break;
          }
      }
      
      return relayStatus;
    }

    void relayOn(int ch) {
      if (ch == 1) {
        if (storage->getRelayPin(1) > -1) {
            Serial.println("relay1 On");
          this->relay1->on();
        }
      } else if (ch == 2) {
        if (storage->getRelayPin(2) > -1) {
             Serial.println("relay2 On");
          this->relay2->on();
        }
      } else if (ch == 3) {
        if (storage->getRelayPin(3) > -1) {
             Serial.println("relay3 On");
          this->relay3->on();
        }
      } else if (ch == 4) {
        if (storage->getRelayPin(4) > -1) {
            Serial.println("relay4 On");
          this->relay4->on();
        }
      }
    }

    void relayOff(int ch) {
       if (ch == 1) {
        if (storage->getRelayPin(1) > -1) {
            Serial.println("relay1 Off");
          this->relay1->off();
        }
      } else if (ch == 2) {
        if (storage->getRelayPin(2) > -1) {
             Serial.println("relay2 Off");
          this->relay2->off();
        }
      } else if (ch == 3) {
        if (storage->getRelayPin(3) > -1) {
             Serial.println("relay3 Off");
          this->relay3->off();
        }
      } else if (ch == 4) {
        if (storage->getRelayPin(4) > -1) {
            Serial.println("relay4 Off");
          this->relay4->off();
        }
      }
    }

    String getRelayStatusAsString(int ch) {
      bool s = getRelayStatus(ch);
      return s ? "on" : "off";
    }

    uint8_t getSwitchEvent(int ch) {
      if (ch == 1) {
        return this->sw1->getEvent();
      } else if (ch == 2) {
        return this->sw2->getEvent();
      } else if (ch == 3) {
        return this->sw3->getEvent();
      } else if (ch == 4) {
        return this->sw4->getEvent();
      }
    }



    uint8_t getLed() {
      return this->storage->getLedPin();
    }
};
#endif /* Sonoff_h */
