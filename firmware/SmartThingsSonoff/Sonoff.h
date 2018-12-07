
#ifndef Sonoff_h
#define Sonoff_h

#include "relay.h"
#include "switch.h"

#define ARDUINO_ESP8266_ESP01
#define SONOFFSWITCH
//#define SONOFFPOW

// Devicedefinitions

#ifdef SONOFFSWITCH
#define DEVICE 1
#define SWITCH14
#endif

#ifdef SONOFFPOW
#define DEVICE 2
#endif

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

#if DEVICE==1   // SONOFFSWITCH
#define LEDgreen D7
#define RELAYPIN D6
#define GPIO0 D3
#define BUTTON_PIN D5
#endif

#if DEVICE==2   // SONOFFPOW
#define LEDgreen D8
#define RELAYPIN D6
#define GPIO0 D3
#define SEL_PIN  5
#define CF1_PIN  14
#define CF_PIN   13

#include <HLW8012.h>
// Set SEL_PIN to HIGH to sample current
// This is the case for Itead's Sonoff POW, where a
// the SEL_PIN drives a transistor that pulls down
// the SEL pin in the HLW8012 when closed
#define CURRENT_MODE                    HIGH

// These are the nominal values for the resistors in the circuit
#define CURRENT_RESISTOR                0.001
#define VOLTAGE_RESISTOR_UPSTREAM       ( 5 * 470000 ) // Real: 2280k
#define VOLTAGE_RESISTOR_DOWNSTREAM     ( 1000 ) // Real 1.009k

HLW8012 * hlw8012 = new HLW8012();

// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void hlw8012_cf1_interrupt() {
  //Serial.println("hlw8012_cf1_interrupt ");
  hlw8012->cf1_interrupt();
}
void hlw8012_cf_interrupt() {
  hlw8012->cf_interrupt();
}

// Library expects an interrupt on both edges
void setInterrupts() {
  detachInterrupt(CF1_PIN);
  attachInterrupt(CF1_PIN, hlw8012_cf1_interrupt, CHANGE);
  detachInterrupt(CF_PIN);
  attachInterrupt(CF_PIN, hlw8012_cf_interrupt, CHANGE);
}


#endif



static const uint8_t RELAY = RELAYPIN; // MTDI
static const uint8_t SWITCH = D3; // GPIO0
static const uint8_t LED = LEDgreen; // MTCK

class Sonoff
{

  private:
#if DEVICE == 2
    void unblockingDelay(unsigned long mseconds) {
      unsigned long timeout = millis();
      while ((millis() - timeout) < mseconds) delay(1);
    }
#endif
#ifdef SWITCH14
    boolean buttonState = false;
    boolean buttonChanged = false;
#endif
  public:
    Relay relay = Relay( RELAY, 0.4, 500, 25000);
    Switch sw =  Switch( SWITCH, 2, 5);

    Sonoff(void) {

    }

    void loop() {
      relay.loop();
      sw.loop();

      }

    void setup() {
#if DEVICE==2
      hlw8012->begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, true);
      hlw8012->setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);
      setInterrupts();
      Serial.println ( "hlw8012 installed" );
#endif
#ifdef SWITCH14
      pinMode(BUTTON_PIN, INPUT);
#endif
    }

#if DEVICE==1
    boolean isPow() {
      return false;
    }

#ifdef SWITCH14
    boolean IsButtonOn() {
      buttonState = digitalRead(BUTTON_PIN);
      return !(buttonState == HIGH);
    }
#else
    boolean IsButtonOn() {
      return false;     
    }
#endif
    double getCurrent() {
      return 0;
    }

    unsigned int getVoltage() {
      return 0;
    }
    unsigned int getActivePower() {
      return 0;
    }
    unsigned int getApparentPower() {
      return 0;
    }
    double getPowerFactor() {
      return 0;
    }
    unsigned int getReactivePower() {
      return 0;
    }
    unsigned long getEnergy() {
      return 0;
    }

    unsigned long getCurrentMultiplier() {
      return 0;
    }

    unsigned long getVoltageMultiplier() {
      return 0;
    }

    unsigned long getPowerMultiplier() {
      return 0;
    }

    void calibrate() {

    }
#endif
#if DEVICE == 2
    boolean isPow() {
      return true;
    }


    boolean IsButtonOn() {
      return false;
    }

    double getCurrent() {
      return hlw8012->getCurrent();
    }

    unsigned int getVoltage() {
      return  hlw8012->getVoltage();

    }
    unsigned int getActivePower() {
      return hlw8012->getActivePower();
    }
    unsigned int getApparentPower() {
      return hlw8012->getApparentPower();
    }
    double getPowerFactor() {
      return 100 * hlw8012->getPowerFactor();
    }
    unsigned int getReactivePower() {
      return hlw8012->getReactivePower();
    }
    unsigned long getEnergy() {
      return hlw8012->getEnergy();
    }

    unsigned long getCurrentMultiplier() {
      return hlw8012->getCurrentMultiplier();
    }

    unsigned long getVoltageMultiplier() {
      return hlw8012->getVoltageMultiplier();
    }

    unsigned long getPowerMultiplier() {
      return hlw8012->getPowerMultiplier();
    }

    void calibrate() {

      // Let's first read power, current and voltage
      // with an interval in between to allow the signal to stabilise:

      hlw8012->getActivePower();

      hlw8012->setMode(MODE_CURRENT);
      unblockingDelay(2000);
      hlw8012->getCurrent();

      hlw8012->setMode(MODE_VOLTAGE);
      unblockingDelay(2000);
      hlw8012->getVoltage();

      // Calibrate using a 60W bulb (pure resistive) on a 230V line
      hlw8012->expectedActivePower(60.0);
      hlw8012->expectedVoltage(230.0);
      hlw8012->expectedCurrent(60.0 / 230.0);

      // Show corrected factors
      Serial.print("[HLW] New current multiplier : "); Serial.println(hlw8012->getCurrentMultiplier());
      Serial.print("[HLW] New voltage multiplier : "); Serial.println(hlw8012->getVoltageMultiplier());
      Serial.print("[HLW] New power multiplier   : "); Serial.println(hlw8012->getPowerMultiplier());
      Serial.println();

    }
#endif

};
#endif /* Sonoff_h */
