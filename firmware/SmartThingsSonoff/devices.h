#ifndef Device_h
#define Device_h
#include "Sonoff.h"
#include "SamsungSmartThings.h"
#include "CSE7766.h"

#define SSDP_CSE776_DEVICE_TYPE "urn:sonoff:device:vzakharchenko:e:1"
// #define SSDP_ONECHANEL_DEVICE_TYPE "urn:sonoff:device:vzakharchenko:1"


#ifdef SSDP_CSE776_DEVICE_TYPE
#define SSDP_DEVICE_TYPE SSDP_CSE776_DEVICE_TYPE
#define CSE7766_TYPE

#else
#define SSDP_DEVICE_TYPE SSDP_ONECHANEL_DEVICE_TYPE
#endif //SSDP_CSE776_DEVICE_TYPE


#define SONOFF_BASIC 0
#define SONOFF_POW 1
#define SONOFF_RF 2
#define SONOFF_TH 3
#define SONOFF_SV 4
#define SLAMPHER 5
#define S20 6
#define SONOFF_TOUCH 7
#define SONOFF_POW_R2 8
#define SONOFF_S31 9
#define SONOFF_S31_LITE 10
#define ORVIBO_B25 11

#define SONOFF_T1_1CH 12
#define NODEMCU_LOLIN 13
#define D1_MINI_RELAYSHIELD 14
#define YJZK_SWITCH_1CH 15
#define WORKCHOICE_ECOPLUG 16
#define OPENENERGYMONITOR_MQTT_RELAY 17
#define WION_50055 18
#define EXS_WIFI_RELAY_V31 19
#define XENON_SM_PW702U 20
#define ISELECTOR_SM_PW702 21
#define ISELECTOR_SM_PW702U 22
#define KMC_70011 23
#define EUROMATE_WIFI_STECKER_SCHUKO 24


class DeviceHandler
{
  private:
    SmartThings* smartThings;
#ifdef CSE7766_TYPE
    CSE7766 powR2;
    unsigned long mLastTime = 0;
#define POW_R2_UPDATE_TIME                     5000
#endif //CSE7766
  public:
    DeviceHandler(SmartThings *smartThings) {
      this->smartThings = smartThings;
    }
    void begin() {
#ifdef CSE7766_TYPE
      powR2.setRX(1);
      powR2.begin(); // will initialize serial to 4800 bps
#endif //CSE7766_TYPE;
    }
    void loop() {
#ifdef CSE7766_TYPE


      if ((millis() - mLastTime) >= POW_R2_UPDATE_TIME) {

        // Time
        mLastTime = millis();

        // read CSE7766
        powR2.handle();
        yield();
        smartThings->sendDirectlyData(
          String( "{\"cse7766\":{") +
          String("\"voltage\":\"") + String(powR2.getVoltage()) + String( "\",") +
          String("\"current\":\"") + String(powR2.getCurrent()) + String("\",") +
          String("\"activePower\":\"" ) + String(powR2.getActivePower()) + String("\",") +
          String("\"apparentPower\":\"") + String(powR2.getApparentPower()) + String("\",") +
          String("\"reactivePower\":\"") + String(powR2.getReactivePower()) + String("\",") +
          String("\"energy\":\"") + String(powR2.getEnergy()) + String("\"") +
          String("}}")
        );

        Serial.println("Voltage " + String(powR2.getVoltage()));
        Serial.println("Current " + String(powR2.getCurrent()));
        Serial.println("ActivePower " + String(powR2.getActivePower()));
        Serial.println("ApparentPower " + String(powR2.getApparentPower()));
        Serial.println("ReactivePower " + String(powR2.getReactivePower()));
        Serial.println("PowerFactor " + String(powR2.getPowerFactor()));
        Serial.println("Energy " + String(powR2.getEnergy()));

      }
#endif //CSE7766_TYPE

    }
};

class Device
{

  private:
    int relayPin = D6;
    int switchPin = D3;
    int ledPin = D7;
    int ledPinInverse = 1;
  public:
    // SWITCH
    Device(int device) {
      switch (device) {
        case WORKCHOICE_ECOPLUG: {
            switchPin = D7;
            break;
          }
        case SONOFF_BASIC:
        case SONOFF_POW:
        case SONOFF_RF:
        case SONOFF_TH:
        case SONOFF_SV:
        case SLAMPHER:
        case S20:
        case SONOFF_TOUCH:
        case SONOFF_POW_R2:
        case SONOFF_S31:
        case SONOFF_S31_LITE:
        case YJZK_SWITCH_1CH:
        case D1_MINI_RELAYSHIELD:
        case NODEMCU_LOLIN:
        case SONOFF_T1_1CH:
        case KMC_70011:
        case OPENENERGYMONITOR_MQTT_RELAY:
        case EXS_WIFI_RELAY_V31:
          {
            switchPin = D3;
            break;
          }
        case EUROMATE_WIFI_STECKER_SCHUKO:
        case ORVIBO_B25: {
            switchPin = D5;
            break;
          }
        case XENON_SM_PW702U:
        case ISELECTOR_SM_PW702:
        case WION_50055: {
            switchPin = 13;
            break;
          }
        default: {
            switchPin = D3;
            break;
          }
      }
      //RELAY PIN
      switch (device) {
        case SONOFF_BASIC:
        case SONOFF_POW:
        case SONOFF_RF:
        case SONOFF_TH:
        case SONOFF_SV:
        case SLAMPHER:
        case S20:
        case SONOFF_TOUCH:
        case SONOFF_POW_R2:
        case SONOFF_S31:
        case SONOFF_S31_LITE:
        case SONOFF_T1_1CH: {
            relayPin = D6;
            break;
          }
        case D1_MINI_RELAYSHIELD:
        case EUROMATE_WIFI_STECKER_SCHUKO:
        case ORVIBO_B25: {
            relayPin = D1;
            break;
          }
        case YJZK_SWITCH_1CH:
        case OPENENERGYMONITOR_MQTT_RELAY:
        case XENON_SM_PW702U:
        case ISELECTOR_SM_PW702:
        case NODEMCU_LOLIN: {
            relayPin = 12;
            break;
          }
        case WION_50055:
        case WORKCHOICE_ECOPLUG: {
            relayPin = 15;
            break;
          }
        case EXS_WIFI_RELAY_V31: {
            relayPin = 13;
            break;
          }
        case KMC_70011: {
            relayPin = 14;
            break;
          }
        default: {
            relayPin = D3;
            break;
          }
      }
      // ledPin
      switch (device) {
        case SONOFF_BASIC:
        case SONOFF_RF:
        case SONOFF_TH:
        case SONOFF_SV:
        case SLAMPHER:
        case S20:
        case SONOFF_TOUCH:
        case SONOFF_POW_R2:
        case SONOFF_S31:
        case SONOFF_S31_LITE:
        case YJZK_SWITCH_1CH:
        case KMC_70011:
        case SONOFF_T1_1CH: {
            ledPin = D7;
            break;
          }
        case SONOFF_POW: {
            ledPin = D8;
            break;
          }
        case ORVIBO_B25: {
            ledPin = D6;
            break;
          }
        case OPENENERGYMONITOR_MQTT_RELAY: {
            ledPin = 16;
            break;
          }
        case D1_MINI_RELAYSHIELD:
        case WORKCHOICE_ECOPLUG:
        case WION_50055:
        case EXS_WIFI_RELAY_V31:
        case NODEMCU_LOLIN: {
            ledPin = 2;
            break;
          }
        case ISELECTOR_SM_PW702:
        case EUROMATE_WIFI_STECKER_SCHUKO:
        case XENON_SM_PW702U: {
            ledPin = 4;
            break;
          }
        default: {
            ledPin = D7;
            break;
          }
      }
      // ledPinInverse
      switch (device) {
        case WORKCHOICE_ECOPLUG:
        case OPENENERGYMONITOR_MQTT_RELAY:
        case YJZK_SWITCH_1CH: {
            ledPinInverse = 0;
            break;
          }
        default: {
            ledPinInverse = 1;
            break;
          }
      }
    }

    int getRelayPin() {
      return this->relayPin;
    }

    int getSwitchPin() {
      return this->switchPin;
    }
    int getLedPin() {
      return this->ledPin;
    }

    int getLedPinInverse() {
      return this->ledPinInverse;
    }


};

String deviceJSON(int id, String label) {
  String r = String( "{\"id\":\"") + String(id) + String("\",\"name\":\"") + label + String("\"}");
  return r;
}
String getDeviceJson() {
  return "[" +
         deviceJSON(SONOFF_BASIC, "SONOFF BASIC") + "," +
         deviceJSON(SONOFF_POW, "SONOFF POW") + "," +
         deviceJSON(SONOFF_RF, "SONOFF RF") + "," +
         deviceJSON(SONOFF_TH, "SONOFF TH") + "," +
         deviceJSON(SONOFF_SV, "SONOFF SV") + "," +
         deviceJSON(SLAMPHER, "SLAMPHER") + "," +
         deviceJSON(S20, "S20") + "," +
         deviceJSON(SONOFF_TOUCH, "SONOFF TOUCH") + "," +
         deviceJSON(SONOFF_POW_R2, "SONOFF POW R2") + "," +
         deviceJSON(SONOFF_S31, "SONOFF S31") + "," +
         deviceJSON(SONOFF_T1_1CH, "SONOFF T1 1CH") + "," +
         deviceJSON(ORVIBO_B25, "ORVIBO B25") + "," +
         deviceJSON(SONOFF_T1_1CH, "SONOFF T1 1CH") + "," +
         deviceJSON(NODEMCU_LOLIN, "NODEMCU LOLIN") + "," +
         deviceJSON(D1_MINI_RELAYSHIELD, "D1 MINI RELAYSHIELD") + "," +
         deviceJSON(YJZK_SWITCH_1CH, "YJZK SWITCH 1CH") + "," +
         deviceJSON(WORKCHOICE_ECOPLUG, "WORKCHOICE ECOPLUG") + "," +
         deviceJSON(OPENENERGYMONITOR_MQTT_RELAY, "OPENENERGYMONITOR MQTT RELAY") + "," +
         deviceJSON(WION_50055, "WION 50055") + "," +
         deviceJSON(EXS_WIFI_RELAY_V31, "EXS WIFI RELAY V31") + "," +
         deviceJSON(XENON_SM_PW702U, "XENON SM PW702U") + "," +
         deviceJSON(ISELECTOR_SM_PW702, "ISELECTOR SM PW702") + "," +
         deviceJSON(ISELECTOR_SM_PW702U, "ISELECTOR SM PW702U") + "," +
         deviceJSON(KMC_70011, "KMC 70011") + "," +
         deviceJSON(EUROMATE_WIFI_STECKER_SCHUKO, "EUROMATE WIFI STECKER SCHUKO") + "," +
         deviceJSON(NODEMCU_LOLIN, "LINGAN SWA1")
         + "]";
}
#endif /* Device_h */
