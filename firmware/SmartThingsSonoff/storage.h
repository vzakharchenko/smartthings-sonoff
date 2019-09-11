// #include "Sonoff.h"

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
      char signature[3];
    } Configuration13106;

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

    Configuration13107 configuration {
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


        if (storageVersion == 106) {
          Configuration13106 readConfiguration {
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
            "BD"
          };
          //  loadStruct(&readConfiguration, sizeof(readConfiguration));
          Serial.println ( "Storage loaded" );
          if (String(readConfiguration.signature) == String("OK")) {
            Serial.println ( "Configuration is Valid: " + String(readConfiguration.signature) + " lastState: " + String(readConfiguration.lastState));
            strcpy(configuration.smartThingsUrl, readConfiguration.smartThingsUrl);
            strcpy(configuration.applicationId, readConfiguration.applicationId);
            strcpy(configuration.accessToken, readConfiguration.accessToken);
            configuration.defaultState = readConfiguration.defaultState;
            configuration.lastState = readConfiguration.lastState;
            configuration.relayPin = readConfiguration.relayPin;
            configuration.switchPin = readConfiguration.switchPin;
            configuration.externalSwitchPin = readConfiguration.externalSwitchPin;
            configuration.ledPin = readConfiguration.ledPin;
            configuration.externalSwitchState = readConfiguration.externalSwitchState;
            configuration.ledPinInverse = readConfiguration.ledPinInverse;
            configuration.seq = 0;
            save();
          } else {
            Serial.println ( "Configuration inValid: " + String(readConfiguration.signature));
          }
        }
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
            "BD"
          };
          loadStruct(&readConfiguration, sizeof(readConfiguration));
          Serial.println ( "Storage loaded" );
          if (String(readConfiguration.signature) == String("OK")) {
            Serial.println ( "Configuration is Valid: " + String(readConfiguration.signature) + " lastState: " + String(readConfiguration.lastState));
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

    int getDefaultState() {
      return configuration.defaultState;
    }

    void setDefaultState(int defaultState) {
      configuration.defaultState = defaultState;
    }

    bool getLastState() {
      return configuration.lastState;
    }

    void setLastState(bool lastState) {
      configuration.lastState = lastState;
    }

    bool isValid() {
      return String(configuration.signature) == String("OK");
    }

    int getRelayPin() {
      return configuration.relayPin;
    }

    void setRelayPin(int relayPin) {
      configuration.relayPin = relayPin;
    }

    int getSwitchPin() {
      return configuration.switchPin;
    }

    void setSwitchPin(int switchPin) {
      configuration.switchPin = switchPin;
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
