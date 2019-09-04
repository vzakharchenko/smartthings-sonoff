#ifndef SmartThings_h
#define SmartThings_h
#include "devices.h"
typedef struct
{
  int state;
  char devName[256];
} SmartThingDevice;


String IpAddress2String(const IPAddress& ipAddress)
{
  String ip = String(ipAddress[0]) + String(".") + \
              String(ipAddress[1]) + String(".") + \
              String(ipAddress[2]) + String(".") + \
              String(ipAddress[3]);
  return ip  ;
}


class SmartThings
{
  private:

    Sonoff* sonoff;
    Storage* storage;
    int changeState(String operation, boolean force) {
      Serial.println ( "changeState:  " + operation + "/" + String(force) );
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {
        HTTPClient http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/" + String(storage->getApplicationId()) + "/" + operation + "?access_token=" + String(storage->getAccessToken());
        Serial.println ( "Starting SmartThings Http " + operation + " : " + url );
        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        client->setInsecure();
        http.begin(*client, url);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST("{\"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\",\"force\":" + String((force ? "true" : "false")) + "}");
        if (httpCode > 199 && httpCode < 301) {
          String payload = http.getString();
          Serial.println ( "payload = " + payload );
          DynamicJsonDocument doc(2048);
          deserializeJson(doc, payload);
          JsonObject root = doc.to<JsonObject>();
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
        HTTPClient http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/" + String(storage->getApplicationId()) + "/current?access_token=" + String(storage->getAccessToken());
        Serial.println ( "Starting SmartThings Http current : " + url );
        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        client->setInsecure();
        http.begin(*client, url);
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\" }");
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

    String getSmartThingsDevices() {
      String payload = "{}";
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {
        HTTPClient http;
        String url = String(storage->getSmartThingsUrl()) + "/api/smartapps/installations/" + String(storage->getApplicationId()) + "/info?access_token=" + String(storage->getAccessToken());
        Serial.println ( "Starting SmartThings Http current : " + url );
        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        client->setInsecure();
        http.begin(*client, url);
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\" }");
        String payloadJSON = http.getString();
        http.end();
        if (payloadJSON != String("")) {
          payload = payloadJSON;
        }

      }
      return payload;

    }

    int off(boolean force) {
      return changeState("off", force);
    }




    boolean smartthingsInit0(String smartThingsUrl) {
      Serial.println ( "Starting SmartThings Init" );
      if (String(storage->getApplicationId()) != String("") &&
          String(storage->getAccessToken()) != String("")) {

        HTTPClient http;
        String url = smartThingsUrl + "/api/smartapps/installations/"
                     + String(storage->getApplicationId())
                     + "/" + "init?access_token=" + String(storage->getAccessToken()
                         + "&ip=" + IpAddress2String( WiFi.localIP())
                         + "&mac=" + String(WiFi.macAddress())
                         + "&relay=" + String(sonoff->getRelay()->isOn() ? "on" : "off")
                                                          );
        Serial.println ( "Sending Http Get " + url );
        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

        client->setInsecure();
        http.begin(*client, url);
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
