

typedef struct
{
  int state;
  char devName[256];
} SmartThingDevice;

class TransportTraits
{
  public:
    virtual ~TransportTraits()
    {
    }

    virtual std::unique_ptr<WiFiClient> create()
    {
      return std::unique_ptr<WiFiClient>(new WiFiClient());
    }

    virtual bool verify(WiFiClient& client, const char* host)
    {
      (void)client;
      (void)host;
      return true;
    }
};

class TLSTraits : public TransportTraits
{
  public:
    TLSTraits()
    {
    }

    std::unique_ptr<WiFiClient> create() override
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored  "-Wdeprecated-declarations"
      return std::unique_ptr<WiFiClient>(new axTLS::WiFiClientSecure());
#pragma GCC diagnostic pop
    }

    bool verify(WiFiClient& client, const char* host) override
    {
      auto wcs = static_cast<axTLS::WiFiClientSecure&>(client);
      return true;
    }
};

String IpAddress2String(const IPAddress& ipAddress)
{
  String ip = String(ipAddress[0]) + String(".") + \
              String(ipAddress[1]) + String(".") + \
              String(ipAddress[2]) + String(".") + \
              String(ipAddress[3]);
  return ip  ;
}

class HTTPClient2 : public HTTPClient
{
  public:
    HTTPClient2()
    {
    }

    bool beginInternal2(String url, const char* expectedProtocol)
    {
      _transportTraits = TransportTraitsPtr(new TLSTraits());
      _port = 443;
      return beginInternal(url, expectedProtocol);
    }
};


class SmartThings
{
  private:

    Sonoff* sonoff;
    Storage* storage;
    int changeState(String operation, boolean force) {
      Serial.println ( "changeState:  " + operation + "/" + String(force) );
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {
        HTTPClient2 http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/" + String(storage->getApplicationId()) + "/" + operation + "?access_token=" + String(storage->getAccessToken());
        Serial.println ( "Starting SmartThings Http " + operation + " : " + url );
        http.beginInternal2(url, "https");
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST("{\"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\",\"force\":" + String((force ? "true" : "false")) + "}");
        if (httpCode > 199 && httpCode < 301) {
          String payload = http.getString();
          Serial.println ( "payload = " + payload );
          DynamicJsonBuffer jsonBuffer;
          JsonObject& root = jsonBuffer.parseObject(payload);
          http.end();
          if (root["relay"] == String("on")) {
            return 1;
          } else if (root["relay"] == String("off")) {
            return 0;
          } else {
            return -1;
          }
        } else {
          http.end();
          return  -1;

        }


      } else {
        Serial.println ( "Empty SmartThings Configuration!!!" );
      }
      if (operation == String("on")) {
        return 1;
      } else {
        return 0;
      }

    }
  public:

    SmartThings(Sonoff *sonoff, Storage* storage ) {
      this->sonoff = sonoff;
      this->storage = storage;
      //this->storage2 = (Storage *) storage;
    }

    int on(boolean force) {
      return changeState("on", force);
    }

    int getSwitchState() {
      return getSmartThingsDevice().state;
    }


    SmartThingDevice getSmartThingsDevice() {
      SmartThingDevice smartThingsDevice{
        -1,
        ""
      };
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {
        HTTPClient2 http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/" + String(storage->getApplicationId()) + "/current?access_token=" + String(storage->getAccessToken());
        Serial.println ( "Starting SmartThings Http current : " + url );
        http.beginInternal2(url, "https");
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\" }");
        String payload = http.getString();
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(payload);
        String status = root["status"];
        String devName = root["name"];
        int stState;
        if (status == "on") {
          stState = 1;
        } else if (status == "off") {
          stState = 0;
        } else {
          stState = 2;
        }
        smartThingsDevice.state = stState;
        devName.toCharArray(smartThingsDevice.devName, devName.length() + 1);
        http.end();
      }

      return smartThingsDevice;
    }

    String getSmartThingsDevices() {
      String payload = "{}";
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {
        HTTPClient2 http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/" + String(storage->getApplicationId()) + "/info?access_token=" + String(storage->getAccessToken());
        Serial.println ( "Starting SmartThings Http current : " + url );
        http.beginInternal2(url, "https");
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\" }");
        payload = http.getString();
        http.end();

      }
      return payload;

    }

    int off(boolean force) {
      return changeState("off", force);
    }

    void incomingCall(boolean start) {
      Serial.println ( "incomingCall " + String(start) );
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {
        HTTPClient2 http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/" + String(storage->getApplicationId()) + "/" + (start ? "incomingCall" : "endCall") + "?access_token=" + String(storage->getAccessToken());
        Serial.println ( "Starting SmartThings Http incomingCall" + String(start) + " : " + url );
        http.beginInternal2(url, "https");
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST("{\"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\"}");
        http.end();
      } else {
        Serial.println ( "Empty SmartThings Configuration!!!" );
      }
    }




    void smartthingsInit() {
      Serial.println ( "Starting SmartThings Init" );
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {

        HTTPClient2 http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/" + String(storage->getApplicationId()) + "/" + "init?access_token=" + String(storage->getAccessToken());
        Serial.println ( "Sending Http Get " + url );
        http.beginInternal2(url, "https");
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"ip\":\""
                  + IpAddress2String( WiFi.localIP())
                  + "\",\"mac\":\""
                  + String(WiFi.macAddress())
                  + "\", \"relay\": \""
                  + String(sonoff->getRelay()->isOn() ? "on" : "off")
                  + "\"}");
        //   http.writeToStream(&Serial);
        http.end();
      } else {
        Serial.println ( "SmartThings Init Skip" );
      }
    }



};
