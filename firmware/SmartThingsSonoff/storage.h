#ifndef Storage_h
#define Storage_h

//#define SSDP_POW_DEVICE_TYPE "urn:sonoff:device:p:1:vassio"
//#define SSDP_CSE776_DEVICE_TYPE "urn:sonoff:device:e:1:vassio"
//#define SSDP_ONECHANEL_DEVICE_TYPE "urn:sonoff:device:1:vassio"
//#define SSDP_2CHANEL_DEVICE_TYPE "urn:sonoff:device:2:vassio"
//#define SSDP_3CHANEL_DEVICE_TYPE "urn:sonoff:device:3:vassio"
#define SSDP_4CHANEL_DEVICE_TYPE "urn:sonoff:device:4:vassio"

#ifdef SSDP_CSE776_DEVICE_TYPE
#define DEFAULT_RELAY1  12
#define DEFAULT_SWITCH1  0
#define DEFAULT_RELAY2 -1
#define DEFAULT_SWITCH2 -1
#define DEFAULT_RELAY3  -1
#define DEFAULT_SWITCH3  -1
#define DEFAULT_RELAY4  -1
#define DEFAULT_SWITCH4  -1
#endif //SSDP_CSE776_DEVICE_TYPE

#ifdef SSDP_POW_DEVICE_TYPE
#define DEFAULT_RELAY1  12
#define DEFAULT_SWITCH1  0
#define DEFAULT_RELAY2  -1
#define DEFAULT_SWITCH2  -1
#define DEFAULT_RELAY3  -1
#define DEFAULT_SWITCH3  -1
#define DEFAULT_RELAY4  -1
#define DEFAULT_SWITCH4  -1
#endif //SSDP_POW_DEVICE_TYPE

#ifdef SSDP_ONECHANEL_DEVICE_TYPE
#define USE_LED_WHEN_SWITCH
#define DEFAULT_RELAY1  12
#define DEFAULT_SWITCH1  0
#define DEFAULT_RELAY2  -1
#define DEFAULT_SWITCH2  -1
#define DEFAULT_RELAY3  -1
#define DEFAULT_SWITCH3  -1
#define DEFAULT_RELAY4  -1
#define DEFAULT_SWITCH4  -1
#endif //SSDP_ONECHANEL_DEVICE_TYPE

#ifdef SSDP_2CHANEL_DEVICE_TYPE
#define DEFAULT_RELAY1  12
#define DEFAULT_SWITCH1  0
#define DEFAULT_RELAY2  5
#define DEFAULT_SWITCH2  9
#define DEFAULT_RELAY3  -1
#define DEFAULT_SWITCH3  -1
#define DEFAULT_RELAY4  -1
#define DEFAULT_SWITCH4  -1
#endif //SSDP_2CHANEL_DEVICE_TYPE

#ifdef SSDP_3CHANEL_DEVICE_TYPE
#define DEFAULT_RELAY1  12
#define DEFAULT_SWITCH1  0
#define DEFAULT_RELAY2  5
#define DEFAULT_SWITCH2  9
#define DEFAULT_RELAY3  4
#define DEFAULT_SWITCH3  10
#define DEFAULT_RELAY4  -1
#define DEFAULT_SWITCH4  -1
#endif //SSDP_3CHANEL_DEVICE_TYPE

#ifdef SSDP_4CHANEL_DEVICE_TYPE
#define DEFAULT_RELAY1  12
#define DEFAULT_SWITCH1  0
#define DEFAULT_RELAY2  5
#define DEFAULT_SWITCH2  9
#define DEFAULT_RELAY3  4
#define DEFAULT_SWITCH3  10
#define DEFAULT_RELAY4  15
#define DEFAULT_SWITCH4  14
#endif //SSDP_4CHANEL_DEVICE_TYPE

class Storage
{
  private:

    String ssid;
    String password;

    typedef struct
    {
      byte package;
      int storageVersion;
    } ConfigurationVersion;

    typedef struct
    {
      byte package;
      int storageVersion;
      char smartThingsUrl[128];
      char applicationId[128];
      char accessToken[128];
      int  defaultState;// boot state of relay: 0-off,1-on,2-last,3-smartthings
      bool lastState;// last state of relay
      int relayPin;
      int switchPin;
      int externalSwitchPin;
      int ledPin;
      int ledPinInverse;
      int externalSwitchState;
      char callback[256];
      int seq;
      char hubHost[256];
      int hubPort;
      char signature[3];
    } Configuration13107;

    typedef struct
    {
      byte package;
      int storageVersion;
      char smartThingsUrl[128];
      char applicationId[128];
      char accessToken[128];
      int  defaultState1;// boot state of relay: 0-off,1-on,2-last,3-smartthings
      int  defaultState2;// boot state of relay: 0-off,1-on,2-last,3-smartthings
      int  defaultState3;// boot state of relay: 0-off,1-on,2-last,3-smartthings
      int  defaultState4;// boot state of relay: 0-off,1-on,2-last,3-smartthings
      bool lastState1;// last state of relay
      bool lastState2;// last state of relay
      bool lastState3;// last state of relay
      bool lastState4;// last state of relay
      int relay1Pin;
      int switch1Pin;
      int relay2Pin;
      int switch2Pin;
      int relay3Pin;
      int switch3Pin;
      int relay4Pin;
      int switch4Pin;
      int externalSwitchPin;
      int ledPin;
      int ledPinInverse;
      int externalSwitchState;
      char callback[256];
      int seq;
      char hubHost[256];
      int hubPort;
      char signature[3];
    } Configuration13108;

    Configuration13108 configuration {
      13,
      108,
      "",
      "",
      "",
      0,
      0,
      0,
      0,
      false,
      false,
      false,
      false,
      DEFAULT_RELAY1,
      DEFAULT_SWITCH1,
      DEFAULT_RELAY2,
      DEFAULT_SWITCH2,
      DEFAULT_RELAY3,
      DEFAULT_SWITCH3,
      DEFAULT_RELAY4,
      DEFAULT_SWITCH4,
      -1,
      13,
      1,
      LOW,
      "",
      0,
      "",
      -1,
      "OK"
    };

  protected:
    void storeStruct(void *data_source, size_t size)
    {
      EEPROM.begin(size * 2);
      for (size_t i = 0; i < size; i++)
      {
        char data = ((char *)data_source)[i];
        EEPROM.write(i, data);
      }
      EEPROM.commit();
      EEPROM.end();
    }

    void loadStruct(void *data_dest, size_t size)
    {
      EEPROM.begin(size * 2);
      for (size_t i = 0; i < size; i++)
      {
        char data = EEPROM.read(i);
        ((char *)data_dest)[i] = data;
      }
      EEPROM.end();
    }
  public:

    Storage() {

    }

    Storage(const char* ssid, const char* password) {
      this->ssid = String(ssid);
      this->password = String(password);
    }

    void save() {
      storeStruct(&configuration, sizeof(configuration));
    }

    void load() {
      Serial.println ( "start loading storage..." );


      boolean packageVersion = 0;
      loadStruct(&packageVersion,  1);
      Serial.println ( "package version " + String(packageVersion) );
      if (packageVersion = 13) {
        ConfigurationVersion configurationVersion;
        loadStruct(&configurationVersion, sizeof(configurationVersion));
        int storageVersion = configurationVersion.storageVersion;
        Serial.println ( "storage version " + String(storageVersion) );


        if (storageVersion == 107) {
          Configuration13107 readConfiguration {
            13,
            107,
            "",
            "",
            "",
            0,
            false,
            12,
            0,
            -1,
            13,
            1,
            LOW,
            "",
            0,
            "",
            -1,
            "OK"
          };
          loadStruct(&readConfiguration, sizeof(readConfiguration));
          Serial.println ( "Storage loaded" );
          if (String(readConfiguration.signature) == String("OK")) {
            Serial.println ( "Configuration is Valid: " + String(readConfiguration.signature) + " lastState: " + String(readConfiguration.lastState));
            strcpy(configuration.smartThingsUrl, readConfiguration.smartThingsUrl);
            strcpy(configuration.applicationId, readConfiguration.applicationId);
            strcpy(configuration.accessToken, readConfiguration.accessToken);
            configuration.defaultState1 = readConfiguration.defaultState;
            configuration.lastState1 = readConfiguration.lastState;
            configuration.relay1Pin = readConfiguration.relayPin;
            configuration.switch1Pin = readConfiguration.switchPin;
            configuration.relay2Pin = -1;
            configuration.switch2Pin = -1;
            configuration.relay3Pin = -1;
            configuration.switch3Pin = -1;
            configuration.relay4Pin = -1;
            configuration.switch4Pin = -1;
            configuration.externalSwitchPin = readConfiguration.externalSwitchPin;
            configuration.ledPin = readConfiguration.ledPin;
            configuration.externalSwitchState = readConfiguration.externalSwitchState;
            configuration.ledPinInverse = readConfiguration.ledPinInverse;
            save();
          } else {
            Serial.println ( "Configuration inValid: " + String(readConfiguration.signature));
          }
        }
        if (storageVersion == 108) {
          Configuration13108 readConfiguration {
            13,
            108,
            "",
            "",
            "",
            0,
            0,
            0,
            0,
            false,
            false,
            false,
            false,
            DEFAULT_RELAY1,
            DEFAULT_SWITCH1,
            DEFAULT_RELAY2,
            DEFAULT_SWITCH2,
            DEFAULT_RELAY3,
            DEFAULT_SWITCH3,
            DEFAULT_RELAY4,
            DEFAULT_SWITCH4,
            -1,
            13,
            1,
            LOW,
            "",
            0,
            "",
            -1,
            "BD"
          };
          loadStruct(&readConfiguration, sizeof(readConfiguration));
          Serial.println ( "Storage loaded" );
          if (String(readConfiguration.signature) == String("OK")) {
            Serial.println ( "Configuration is Valid: " + String(readConfiguration.signature));
            this->configuration = readConfiguration;
          } else {
            Serial.println ( "Configuration inValid: " + String(readConfiguration.signature));
          }
        }
      } else {
        Serial.println ( "Configuration inValid: Package " + String(packageVersion) + " is not supported!" );
      }

    }

    byte getPackageVersion() {
      return configuration.package;
    }

    int getStorageVersion() {
      return configuration.storageVersion;
    }

    String getSmartThingsUrl() {
      return configuration.smartThingsUrl;
    }

    void setSmartThingsUrl(String smartThingsUrl) {
      smartThingsUrl.toCharArray(configuration.smartThingsUrl, smartThingsUrl.length() + 1);
    }

    String getApplicationId() {
      return configuration.applicationId;
    }

    void setApplicationId(String applicationId) {
      applicationId.toCharArray(configuration.applicationId, applicationId.length() + 1);
    }

    String getAccessToken() {
      return configuration.accessToken;
    }

    void setAccessToken(String accessToken) {
      accessToken.toCharArray(configuration.accessToken, accessToken.length() + 1);
    }

    int getDefaultState(int ch) {
      if (ch == 1) {
        return this->configuration.defaultState1;
      } else if (ch == 2) {
        return this->configuration.defaultState2;
      } else if (ch == 3) {
        return this->configuration.defaultState3;
      } else if (ch == 4) {
        return this->configuration.defaultState4;
      } else {
        return -1;
      }

    }

    void setDefaultState(int ch, int defaultState) {
      if (ch == 1) {
        this->configuration.defaultState1 = defaultState;
      } else if (ch == 2) {
        this->configuration.defaultState2 = defaultState;
      } else if (ch == 3) {
        this->configuration.defaultState3 = defaultState;
      } else if (ch == 4) {
        this->configuration.defaultState4 = defaultState;
      }
    }

    bool getLastState(int ch) {
      if (ch == 1) {
        return this->configuration.lastState1;
      } else if (ch == 2) {
        return this->configuration.lastState2;
      } else if (ch == 3) {
        return this->configuration.lastState3;
      } else if (ch == 4) {
        return this->configuration.lastState4;
      } else {
        return -1;
      }
    }

    void setLastState(int ch, bool lastState) {
      if (ch == 1) {
        this->configuration.lastState1 = lastState;
      } else if (ch == 2) {
        this->configuration.lastState2 = lastState;
      } else if (ch == 3) {
        this->configuration.lastState3 = lastState;
      } else if (ch == 4) {
        this->configuration.lastState4 = lastState;
      }
    }

    bool isValid() {
      return String(configuration.signature) == String("OK");
    }


    int getRelayPin(int ch) {
      if (ch == 1) {
        return this->configuration.relay1Pin;
      } else if (ch == 2) {
        return this->configuration.relay2Pin;
      } else if (ch == 3) {
        return this->configuration.relay3Pin;
      } else if (ch == 4) {
        return this->configuration.relay4Pin;
      } else {
        return -1;
      }

    }

    void setRelayPin(int ch, int relayPin) {
      if (ch == 1) {
        this->configuration.relay1Pin = relayPin;
      } else if (ch == 2) {
        this->configuration.relay2Pin = relayPin;
      } else if (ch == 3) {
        this->configuration.relay3Pin = relayPin;
      } else if (ch == 4) {
        this->configuration.relay4Pin = relayPin;
      }
    }

    int getSwitchPin(int ch) {
      if (ch == 1) {
        return this->configuration.switch1Pin;
      } else if (ch == 2) {
        return this->configuration.switch2Pin;
      } else if (ch == 3) {
        return this->configuration.switch3Pin;
      } else if (ch == 4) {
        return this->configuration.switch4Pin;
      } else {
        return -1;
      }
    }

    void setSwitchPin(int ch, int switchPin) {
      if (ch == 1) {
        this->configuration.switch1Pin = switchPin;
      } else if (ch == 2) {
        this->configuration.switch2Pin = switchPin;
      } else if (ch == 3) {
        this->configuration.switch3Pin = switchPin;
      } else if (ch == 4) {
        this->configuration.switch4Pin = switchPin;
      }
    }

    int getExternalSwitchPin() {
      return configuration.externalSwitchPin;
    }

    void setExternalSwitchPin(int externalSwitchPin) {
      configuration.externalSwitchPin = externalSwitchPin;
    }

    int getLedPin() {
      return configuration.ledPin;
    }

    void setLedPin(int ledPin) {
      configuration.ledPin = ledPin;
    }

    int getLedInverse() {
      return configuration.ledPinInverse;
    }

    void setLedPinInverse(int ledPinInverse) {
      configuration.ledPinInverse = ledPinInverse;
    }

    int getExternalSwitchState() {
      return configuration.externalSwitchState;
    }

    void setExternalSwitchState(int externalSwitchState) {
      configuration.externalSwitchState = externalSwitchState == LOW ? LOW : HIGH;
    }

    String getCallBack() {
      return configuration.callback;
    }

    void setCallBack(String callback) {
      callback.toCharArray(configuration.callback, callback.length() + 1);
    }

    String getHubHost() {
      return configuration.hubHost;
    }

    void setHubHost(String host) {
      host.toCharArray(configuration.hubHost, host.length() + 1);
    }


    int getHubPort() {
      return configuration.hubPort;
    }

    void setHubPort(int port) {
      configuration.hubPort = port;
    }


    int getSeq() {
      return configuration.seq;
    }

    void setSeq(int seq) {
      configuration.seq = seq;
    }

};
#endif /* Storage_h */
