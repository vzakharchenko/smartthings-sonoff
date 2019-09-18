#ifndef SmartThings_h
#define SmartThings_h

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
      return std::unique_ptr<WiFiClient>(new WiFiClientSecure());
#pragma GCC diagnostic pop
    }

    bool verify(WiFiClient& client, const char* host) override
    {
      auto wcs = static_cast<WiFiClientSecure&>(client);
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

    void sendDirectlyData(String action, String data) {
      HTTPClient http;
       Serial.println ( "call CallBack Url " + storage->getCallBack() );
      http.begin(storage->getCallBack());
      http.addHeader("Content-Type", "application/json");
      http.POST("{\"ip\":\""
                + IpAddress2String( WiFi.localIP())
                + "\",\"mac\":\""
                + String(WiFi.macAddress())
                + "\",\"action\":\""
                + String(action)
                + "\",\"data\":"
                + String(data)
                + ", \"relay1\": \""
                + String(sonoff->getRelayStatusAsString(1))
                + "\", \"relay2\": \""
                + String(sonoff->getRelayStatusAsString(2))
                + "\", \"relay3\": \""
                + String(sonoff->getRelayStatusAsString(3))
                + "\", \"relay4\": \""
                + String(sonoff->getRelayStatusAsString(4))
                + "\"}");
      http.end();
    }

  public:

    SmartThings(Sonoff *sonoff, Storage* storage ) {
      this->sonoff = sonoff;
      this->storage = storage;
      //this->storage2 = (Storage *) storage;
    }

    void updateStates() {
      sendDirectlyData("changeState", "{}");
    }

    int getSwitchState(int ch) {
      return getSmartThingsDeviceState(ch).state;
    }


    SmartThingDevice getSmartThingsDeviceState(int ch) {
      SmartThingDevice smartThingsDevice{
        -1,
        ""
      };
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {
        HTTPClient2 http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/"
                     + String(storage->getApplicationId())
                     + "/get/info?access_token=" + String(storage->getAccessToken()
                         + "&ip=" + IpAddress2String( WiFi.localIP())
                         + "&mac=" + String(WiFi.macAddress())
                         + "&ch=" + String(ch));
        Serial.println ( "Starting SmartThings Http current : " + url );
        http.beginInternal2(url, "https");
        http.addHeader("Content-Type", "application/json");
        http.GET();
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        JsonObject root = doc.to<JsonObject>();
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
    //
    //    String getSmartThingsDevice(int ch) {
    //      String payload = "{}";
    //      if (String(storage->getApplicationId()) != String("") &&
    //          String(storage->getAccessToken()) != String("")) {
    //        HTTPClient2 http;
    //        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/"
    //                     + String(storage->getApplicationId())
    //                     + "/get/info?access_token=" + String(storage->getAccessToken()
    //                         + "&ip=" + IpAddress2String( WiFi.localIP()) +
    //                         + "&ch=" + String(ch) +
    //                         + "&mac=" + String(WiFi.macAddress()));
    //        Serial.println ( "Starting SmartThings Http current : " + url );
    //        http.beginInternal2(url, "https");
    //        http.addHeader("Content-Type", "application/json");
    //        http.GET();
    //        String payloadJSON = http.getString();
    //        http.end();
    //        if (payloadJSON != String("")) {
    //          payload = payloadJSON;
    //        }
    //
    //      }
    //      return payload;
    //
    //    }

    //    String subscribe() {
    //      String payload = "{}";
    //      if (String(storage->getApplicationId()) != String("") &&
    //          String(storage->getAccessToken()) != String("")) {
    //        HTTPClient2 http;
    //        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/"
    //                     + String(storage->getApplicationId())
    //                     + "/get/subscribe?access_token=" + String(storage->getAccessToken()
    //                         + "&ip=" + IpAddress2String( WiFi.localIP())
    //                         + "&mac=" + String(WiFi.macAddress()));
    //        Serial.println ( "Starting SmartThings Http current : " + url );
    //        http.beginInternal2(url, "https");
    //        http.addHeader("Content-Type", "application/json");
    //        http.GET();
    //        String payloadJSON = http.getString();
    //        http.end();
    //        if (payloadJSON != String("")) {
    //          payload = payloadJSON;
    //        }
    //
    //      }
    //      return payload;
    //
    //    }

    void subscribe() {
      sendDirectlyData("subscribe", "{}");
    }

    void sendCSE7766Data(String data) {
      sendDirectlyData("CSE7766", data);
    }


    boolean smartthingsInit0(String smartThingsUrl) {
      Serial.println ( "Starting SmartThings Init" );
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {

        HTTPClient2 http;
        String url = smartThingsUrl + "/api/smartapps/installations/"
                     + String(storage->getApplicationId())
                     + "/" + "init?access_token=" + String(storage->getAccessToken()
                         + "&ip=" + IpAddress2String( WiFi.localIP())
                         + "&mac=" + String(WiFi.macAddress())
                         + "&relay1=" + String(sonoff->getRelayStatusAsString(1)) +
                         + "&relay2=" + String(sonoff->getRelayStatusAsString(2)) +
                         + "&relay3=" + String(sonoff->getRelayStatusAsString(3)) +
                         + "&relay4=" + String(sonoff->getRelayStatusAsString(4))
                                                          );
        Serial.println ( "Sending Http Get " + url );
        http.beginInternal2(url, "https");
        http.setTimeout(10000);
        http.addHeader("Content-Type", "application/json");/*
        http.POST("{\"ip\":\""
                  + IpAddress2String( WiFi.localIP())
                  + "\",\"supportedDevices\":"
                  + getDeviceJson()
                  + ",\"mac\":\""
                  + String(WiFi.macAddress())
                  + "\", \"relay\": \""
                  + String(sonoff->getRelay()->isOn() ? "on" : "off")
                  + "\"}");*/
        http.GET();
        //   http.writeToStream(&Serial);
        Serial.println ( "success connected" );
        String payload = http.getString();
        http.end();
        return payload == String("OK");
      } else {
        Serial.println ( "SmartThings Init Skip" );
        return false;
      }
    }

    void smartthingsInit() {
      if (smartthingsInit0("https://graph-eu01-euwest1.api.smartthings.com")) {
        storage->setSmartThingsUrl("https://graph-eu01-euwest1.api.smartthings.com");
      } else if (smartthingsInit0("https://graph-na04-useast2.api.smartthings.com")) {
        storage->setSmartThingsUrl("https://graph-na04-useast2.api.smartthings.com");
      } else if (smartthingsInit0("https://graph.api.smartthings.com")) {
        storage->setSmartThingsUrl("https://graph.api.smartthings.com");
      } else if (smartthingsInit0("https://graph-na02-useast1.api.smartthings.com")) {
        storage->setSmartThingsUrl("https://graph-na02-useast1.api.smartthings.com");
      } else if (smartthingsInit0("https://graph-ap02-apnortheast2.api.smartthings.com")) {
        storage->setSmartThingsUrl("https://graph-ap02-apnortheast2.api.smartthings.com");
      } else {
        storage->setSmartThingsUrl("unknown");
      }
      storage->save();
    }


};


#endif /* SmartThings_h */
