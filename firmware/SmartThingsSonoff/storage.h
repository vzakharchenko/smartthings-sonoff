
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
      char smartThingsUrl[128];
      char applicationId[128];
      char accessToken[128];
      int  defaultState;
      bool  lastState;
      char signature[3];
    } Configuration12;

    typedef struct
    {
      byte package;
      int storageVersion;
      char smartThingsUrl[128];
      char applicationId[128];
      char accessToken[128];
      int  defaultState;
      bool  lastState;
      char signature[3];
    } Configuration13101;

    Configuration13101 configuration {
      13,
      101,
      "",
      "",
      "",
      0,
      false,
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
      if (packageVersion == 12) {

        Configuration12 readConfiguration{
          12,
          "",
          "",
          "",
          0,
          false,
          "BD"
        };
        loadStruct(&readConfiguration, sizeof(readConfiguration));
        Serial.println ( "Storage loaded" );
        if (String(readConfiguration.signature) == String("OK")) {
          Serial.println ( "Configuration is Valid: " + String(readConfiguration.signature) + " lastState: " + String(readConfiguration.lastState));
          strcpy(configuration.smartThingsUrl, readConfiguration.smartThingsUrl);
          strcpy(configuration.applicationId, readConfiguration.applicationId);
          strcpy(configuration.accessToken, readConfiguration.accessToken);
          configuration.defaultState = readConfiguration.defaultState;
          configuration.lastState = readConfiguration.lastState;
          save();
        } else {
          Serial.println ( "Configuration inValid: " + String(readConfiguration.signature));
        }
      } else if (packageVersion = 13) {
        ConfigurationVersion configurationVersion;
        loadStruct(&configurationVersion, sizeof(configurationVersion));
        int storageVersion = configurationVersion.storageVersion;
        Serial.println ( "storage version " + String(storageVersion) );
        if (storageVersion == 101) {
          Configuration13101 readConfiguration {
            13,
            101,
            "",
            "",
            "",
            0,
            false,
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

};
