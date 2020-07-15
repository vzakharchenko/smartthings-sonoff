#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Ticker.h>
#include "SSDP.h"
#include "storage.h"
#include "Sonoff.h"
#include "SamsungSmartThings.h"
#include "devices.h"
#include "keycloak.h"



const char *ssid = "";
const char *password = "";

const String ssdpDeviceType = SSDP_DEVICE_TYPE;

unsigned long previousMillis = 0;
const long interval = 2000;

int nexSeq = 0;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
SSDPClass2 SSDP2;

WiFiManager wifiManager;
Storage storage;
Sonoff sonoff(&storage);
SmartThings smartThings(&sonoff, &storage );
DeviceHandler deviceHandler(&smartThings, &sonoff);
Keycloak keycloak(80);
Ticker ticker;



void tick()
{
  //toggle state
  int state = digitalRead(sonoff.getLed());
  digitalWrite(sonoff.getLed(), !state);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}


void switchOff(int ch) {
  switchOff(ch, false);
}

void relayOn(int ch) {
  sonoff.relayOn(ch);
#ifdef SSDP_ONECHANEL_DEVICE_TYPE
  digitalWrite ( sonoff.getLed(), storage.getLedInverse() == 0 ? 1 : 0 );
#endif //SSDP_ONECHANEL_DEVICE_TYPE
}



void relayOff(int ch) {
  sonoff.relayOff(ch);
#ifdef SSDP_ONECHANEL_DEVICE_TYPE
  digitalWrite ( sonoff.getLed(), storage.getLedInverse() == 0 ? 0 : 1 );
#endif //SSDP_ONECHANEL_DEVICE_TYPE
}


void sw(int ch, boolean relayState) {
  Serial.println ( "sw " + String(ch) + " " + String(relayState) );
  if (relayState) {
    relayOn(ch);
  } else {
    relayOff(ch);
  }
  storage.setLastState(ch, relayState);
  storage.save();
}

void switchOn(int ch, boolean force) {
  if (force || !sonoff.getRelayStatus(ch)) {
    sw(ch, true);

  }

  smartThings.updateStates();

}

void switchOff(int ch , boolean force) {
  if (force || (sonoff.getRelayStatus(ch))) {
    sw(ch, false);

  }

  smartThings.updateStates();
}


void cors () {
  String origin = server.arg("origin");
  server.sendHeader("Access-Control-Allow-Origin", String(origin));
  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Credentials", "true");
  server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
}

void handleSettings () {
  //String smartThingsUrl = server.arg("smartThingsUrl");
  String applicationId = server.arg("applicationId");
  String accessToken = server.arg("accessToken");
  String defaultStateString = server.arg("defaultState");
  String deviceTypeString = server.arg("deviceType");
  String externalSwitchPinString = server.arg("externalSwitchPin");
  String externalSwitchStateString = server.arg("externalSwitchState");
  String callback = server.arg("callback");
  String hubHost = server.arg("hub_host");
  String hubPort = server.arg("hub_Port");
  String channelString =  server.arg("ch");

  if (applicationId != String("")) {
    storage.setApplicationId(applicationId);
  }
  if (accessToken != String("")) {
    storage.setAccessToken(accessToken);
  }

  if (deviceTypeString != String("")) {
    int value = deviceTypeString.toInt();
    Device device(value);
    storage.setRelayPin(1, device.getRelayPin(1));
    storage.setRelayPin(2, device.getRelayPin(2));
    storage.setRelayPin(3, device.getRelayPin(3));
    storage.setRelayPin(4, device.getRelayPin(4));
    storage.setSwitchPin(1, device.getSwitchPin(1));
    storage.setSwitchPin(2, device.getSwitchPin(2));
    storage.setSwitchPin(3, device.getSwitchPin(3));
    storage.setSwitchPin(4, device.getSwitchPin(4));
    storage.setLedPin(device.getLedPin());
  }

  if (channelString == String("")) {
    channelString = "1";
  }
  if (channelString != String("")) {
    int ch = channelString.toInt();
    if (defaultStateString != String("")) {
      int defaultState = defaultStateString.toInt();
      storage.setDefaultState(ch, defaultState);
    }
  }


  if (externalSwitchStateString != String("")) {
    int value = externalSwitchStateString.toInt();
    storage.setExternalSwitchState(value);
  }
  if (externalSwitchPinString != String("")) {
    int value = externalSwitchPinString.toInt();
    storage.setExternalSwitchPin(value);
  }
  if (callback != String("")) {
    storage.setCallBack(callback);
  }
  if (hubHost != String("")) {
    storage.setHubHost(hubHost);
  }
  if (hubPort != String("")) {
    int value = hubPort.toInt();
    storage.setHubPort(value);
  }
  storage.save();
  yield();
  sonoff.setup();
  yield();
}

void handleDescription() {
  SSDP2.schema(server.client());
}

void handleSubscription() {
  Serial.println ( "handleSubscription " );
  server.sendHeader("SID", SSDP2.getUuid());
  server.sendHeader("TIMEOUT", "Second-28800");
  server.send ( 200 );
}

void handleOn () {
  int ch = 1;
  String channelString = server.arg("ch");
  if (channelString != String("")) {
    ch = channelString.toInt();
  }
  switchOn(ch, false);
  server.send ( 200, "OK" );
}

void handleOff () {
  int ch = 1;
  String channelString = server.arg("ch");
  if (channelString != String("")) {
    ch = channelString.toInt();
  }
  switchOff(ch, false);
  server.send ( 200,  "OK" );
}

void handleInfo () {
  server.send ( 200, "application/json",
                "{ \"relay1\": \""
                + String(sonoff.getRelayStatusAsString(1))
                + "\" , \"relay2\": \""
                + String(sonoff.getRelayStatusAsString(2))
                + "\" , \"relay3\": \""
                + String(sonoff.getRelayStatusAsString(3))
                + "\" , \"relay4\": \""
                + String(sonoff.getRelayStatusAsString(4))
                + "\",\"uptime\":" +
                String(millis()) +
                ", \"ssid\": \""
                + WiFi.SSID() +
                "\", \"hostName\": \""
                + WiFi.hostname() +
                "\",\"action\":\"info\",\"ip\":\""
                + IpAddress2String( WiFi.localIP())
                + "\", \"mac\":\""
                + String(WiFi.macAddress())
                + "\", \"callback\":\""
                + String(storage.getCallBack())
                + "\", \"applicationId\":\""
                + String(storage.getApplicationId())
                + "\", \"accessToken\":\""
                + String(storage.getAccessToken())
                + "\", \"smartThingsUrl\":\""
                + String(storage.getSmartThingsUrl())
                + "\", \"buttonState\":\""
                + String(sonoff.IsButtonOn())
                + "\",\"supportedDevices\":"
                + getDeviceJson()
                + ", \"versionFirmware\":\""
                + String(storage.getPackageVersion()) + "." + String(storage.getStorageVersion())
                + "\", \"relayPin1\":"
                + String(storage.getRelayPin(1))
                + ", \"switchPin1\":"
                + String(storage.getSwitchPin(1))
                + "\", \"relayPin2\":"
                + String(storage.getRelayPin(2))
                + ", \"switchPin2\":"
                + String(storage.getSwitchPin(2))
                + "\", \"relayPin3\":"
                + String(storage.getRelayPin(3))
                + ", \"switchPin3\":"
                + String(storage.getSwitchPin(3))
                + "\", \"relayPin4\":"
                + String(storage.getRelayPin(4))
                + ", \"switchPin4\":"
                + String(storage.getSwitchPin(4))
                + ", \"externalSwitchPin\":"
                + String(storage.getExternalSwitchPin())
                + ", \"ledPin\":"
                + String(storage.getLedPin())
                + ", \"externalSwitchState\":"
                + String(storage.getExternalSwitchState())
                + ", \"defaultState1\":"
                + String(storage.getDefaultState(1))
                + ", \"defaultState2\":"
                + String(storage.getDefaultState(2))
                + ", \"defaultState3\":"
                + String(storage.getDefaultState(3))
                + ", \"defaultState4\":"
                + String(storage.getDefaultState(4))
                + " }");
  smartThings.smartthingsInit();
}

void handleRoot() {
  if (keycloak.isAuthorized()) {
    server.send ( 200, "application/json", String("{") +
                  String("\"status\":\"OK\"")  +
                  String("}"));
  }
}

void handleReset() {
  wifiManager.resetSettings();
  server.send ( 200, "application/json", String("{") +
                String("\"status\":\"RESET\"")  +
                String("}"));
  Serial.println("Reset..");
  ESP.restart();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

void loopChannel(int ch) {
  //   Serial.println ( "loopChannel  " + String(ch) );
  uint8_t event = sonoff.getSwitchEvent(ch);
  if (event > -1) {
    if ( event == SWITCH_EVENT_ON) {
      Serial.println ( "loopChannel change state " + String(ch) + "  relay status: " + String(sonoff.getRelayStatus(ch)) );
      if (sonoff.getRelayStatus(ch)) {
        Serial.println ( "loopChannel off " + String(ch) );
        switchOff(ch, true);
      }
      else {
        if (!sonoff.getRelayStatus(ch)) {
          Serial.println ( "loopChannel on " + String(ch) );
          switchOn(ch, true);
        }
      }
    }
  }

}

void setupChannel(int i) {
  Serial.println ( "channel setup " + String(i) );
  int defaultState = storage.getDefaultState(i);
  bool lastState = storage.getLastState(i);
  if (defaultState == 1) {
    switchOn(i, true);
  } else if (defaultState == 2) {
    if (lastState) {
      switchOn(i, true);
    } else {
      switchOff(i, true);
    }
  } else if (defaultState == 3) {
    int sthStatus = smartThings.getSwitchState(i);
    if (sthStatus == 0) {
      switchOff(i, true);
    } else if (sthStatus == 1) {
      switchOn(i, true);
    } else {
      if (lastState) {
        switchOn(i, true);
      } else {
        switchOff(i, true);
      }
    }

  } else {
    switchOff(i, true);
  }
}

void setup ( void ) {
  Serial.println ( "begin setup" );
  storage.load();
  pinMode ( sonoff.getLed(), OUTPUT );
  Serial.begin ( 115200 );
  ticker.attach(0.6, tick);

  // wifiManager.resetSettings();//todo
  // wifiManager.setTimeout(180);
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect(ssid, password)) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  ticker.detach();

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( WiFi.SSID() );
  Serial.print ( "IP address: " );
  Serial.println (IpAddress2String( WiFi.localIP()) );
  Serial.print ( "MAC address: " );
  Serial.println (WiFi.macAddress() );

  if ( MDNS.begin ( "Sonoff" ) ) {
    Serial.println ( "MDNS responder not needed" );
  }

  server.on ( "/", handleRoot );
  server.on ( "/config", HTTP_POST, handleSettings );
  server.on ( "/on", handleOn);
  server.on ( "/off", handleOff );
  server.on ( "/info", handleInfo);
  server.on ( "/reset", handleReset);
  server.on("/description.xml", handleDescription);
  server.on ( "/subscribe", handleSubscription );
  server.onNotFound ( handleNotFound );
  keycloak.setup(&server);
  httpUpdater.setup(&server);
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println ( "HTTP server started" );

  SSDP2.setSchemaURL("description.xml");
  SSDP2.setHTTPPort(80);
  SSDP2.setName("SmartThings SonOff");
  SSDP2.setSerialNumber("001788102201");
  SSDP2.setURL("index.html");
  SSDP2.setModelName("SmartThings SonOff");
  SSDP2.setModelNumber("SonOff");
  SSDP2.setModelURL("https://github.com/vzakharchenko/smartthings-sonoff");
  SSDP2.setManufacturer("SonOff");
  SSDP2.setManufacturerURL("https://github.com/vzakharchenko/smartthings-sonoff");
  SSDP2.setDeviceType(ssdpDeviceType);
  SSDP2.begin();
  Serial.println ( "SSDP2 server started" );
  Serial.println("HTTPUpdateServer ready! Open http://" + String(IpAddress2String( WiFi.localIP())) + "/update in your browser");
  sonoff.setup();
  Serial.println ( "subscribe  " );
  smartThings.subscribe();
  digitalWrite ( sonoff.getLed(), storage.getLedInverse() == 0 ? 1 : 0 );
  setupChannel(1);
  setupChannel(2);
  setupChannel(3);
  setupChannel(4);
  int seq = storage.getSeq();
  storage.setSeq(seq + 1);

  deviceHandler.begin();

}

void loop ( void ) {
  server.handleClient();
  yield();
  sonoff.loop();
  //Serial.println ( "sonoff looped  ");
  yield();
  deviceHandler.loop();
  //Serial.println ( "deviceHandler looped  ");
  yield();
  boolean buttonState = sonoff.IsButtonOn();
  //Serial.println ( "buttonState :  " + String(buttonState));
  yield();
  if (sonoff.IsButtonChanged()) {
    if (buttonState) {
      switchOn(1, true);
    } else {
      switchOff(1, true);
    }
  }
  yield();
  loopChannel(1);
  yield();
  // Serial.println ( "loopChannel 1  ");
  loopChannel(2);
  yield();
  // Serial.println ( "loopChannel 2  ");
  loopChannel(3);
  yield();
  //Serial.println ( "loopChannel 3  ");
  loopChannel(4);
  yield();
  // Serial.println ( "loopChannel 4  ");

}
