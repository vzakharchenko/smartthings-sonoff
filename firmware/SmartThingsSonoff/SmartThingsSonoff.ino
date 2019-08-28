#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <Ticker.h>
#include <ESP8266SSDP.h>
#include "storage.h"
#include "Sonoff.h"
#include "SamsungSmartThings.h"

const char *ssid = "";
const char *password = "";

const String ssdpDeviceType = "urn:sonoff:device:vzakharchenko:1";

unsigned long previousMillis = 0;
const long interval = 2000;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;


Storage storage;
Sonoff sonoff(&storage);
SmartThings smartThings(&sonoff, &storage );
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

void switchOff() {
  switchOff(false);
}

void relayOn() {
  sonoff.getRelay()->on();
  digitalWrite ( sonoff.getLed(), 0 );
}



void relayOff() {
  sonoff.getRelay()->off();
  digitalWrite ( sonoff.getLed(), 1 );
}


void sw(boolean relayState) {
  if (relayState) {
    relayOn();
  } else {
    relayOff();
  }
  storage.setLastState(relayState);
  storage.save();
}

void switchOn(boolean force) {

  if (force || !sonoff.getRelay()->isOn()) {
    sw(true);
  }
  smartThings.on(force);

}

void switchOff(boolean force) {
  if (force || (sonoff.getRelay()->isOn())) {
    sw(false);
  }
  smartThings.off(force);
}

int getSwitchState() {
  return smartThings.getSmartThingsDevice().state;
}

void openDoor() {
  relayOff();
  delay(storage.getOpenTimeOut());
  switchOn(true);
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
  String openTimeOutString = server.arg("openTimeOut");
  String intercomCallTimeoutString = server.arg("intercomCallTimeout");
  String gpio14StateString = server.arg("gpio14State");
  if (applicationId != String("")) {
    storage.setApplicationId(applicationId);
  }
  if (accessToken != String("")) {
    storage.setAccessToken(accessToken);
  }

  if (defaultStateString != String("")) {
    int defaultState = defaultStateString.toInt();
    storage.setDefaultState(defaultState);
  }
  if (deviceTypeString != String("")) {
    int value = deviceTypeString.toInt();
    storage.setDeviceType(value);
  }
  if (openTimeOutString != String("")) {
    int value = openTimeOutString.toInt();
    storage.setOpenTimeOut(value);
  }
  if (intercomCallTimeoutString != String("")) {
    int value = intercomCallTimeoutString.toInt();
    storage.setIntercomCallTimeout(value);
  }
  if (gpio14StateString != String("")) {
    int value = gpio14StateString.toInt();
    storage.setGpio14State(value);
  }

  storage.save();
  smartThings.smartthingsInit();
  handleInfo();
}

void handleDescription() {
  SSDP.schema(server.client());
}

void handleOn () {

  switchOn(false);
  server.send ( 200, "application/json", "{ \"relay\": \"on\", \"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\" }" );
}

void handleOpen () {
  openDoor();
  server.send ( 200, "application/json", "{ \"relay\": \"on\", \"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\" }" );
}


void handleOff () {
  switchOff(false);
  server.send ( 200, "application/json", "{ \"relay\": \"off\", \"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\" ,\"mac\":\"" + String(WiFi.macAddress()) + "\"  }" );
}

void handleInfo () {

  String payload = smartThings.getSmartThingsDevices();
  server.send ( 200, "application/json",
                "{ \"relay\": \""
                + String(sonoff.getRelay()->isOn() ? "on" : "off")
                + "\",\"uptime\":" +
                String(millis()) +
                ", \"ssid\": \""
                + WiFi.SSID() +
                "\", \"hostName\": \""
                + WiFi.hostname() +
                "\",\"ip\":\""
                + IpAddress2String( WiFi.localIP())
                + "\", \"mac\":\""
                + String(WiFi.macAddress())
                //                + "\", \"applicationId\":\""
                //                + String(storage.getApplicationId())
                //                + "\", \"accessToken\":\""
                //                + String(storage.getAccessToken())
                //                + "\", \"smartThingsUrl\":\""
                //                + String(storage.getSmartThingsUrl())
                + "\", \"smartthings\":"
                + String(payload)
                + ", \"buttonState\":\""
                + String(sonoff.IsButtonOn())
                + "\", \"versionFirmware\":\""
                + String(storage.getPackageVersion()) + "." + String(storage.getStorageVersion())
                + "\", \"openTimeOut\":"
                + String(storage.getOpenTimeOut())
                + ", \"gpio14State\":"
                + String(storage.getGpio14State())
                + ", \"intercomCallTimeout\":"
                + String(storage.getIntercomCallTimeout())
                + ", \"deviceType\":"
                + String(storage.getDeviceType())
                + ", \"defaultState\":"
                + String(storage.getDefaultState())
                + " }");
}

void handleRoot() {
  String payload = smartThings.getSmartThingsDevices();
  server.send ( 200, "application/json", payload );
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

void setup ( void ) {
  storage.load();
  pinMode ( sonoff.getLed(), OUTPUT );
  Serial.begin ( 9600 );
  ticker.attach(0.6, tick);
  WiFiManager wifiManager;
  // wifiManager.resetSettings();//todo
  wifiManager.setTimeout(180);
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect(ssid, password)) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  ticker.detach();
  digitalWrite ( sonoff.getLed(), 1 );

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
  server.on ( "/open", handleOpen);
  server.on ( "/off", handleOff );
  server.on ( "/info", handleInfo);
  server.on("/description.xml", handleDescription);
  server.onNotFound ( handleNotFound );
  httpUpdater.setup(&server);
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println ( "HTTP server started" );

  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("SmartThings SonOff");
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("index.html");
  SSDP.setModelName("SmartThings SonOff");
  SSDP.setModelNumber("SonOff");
  SSDP.setModelURL("https://github.com/vzakharchenko/smartthings-sonoff");
  SSDP.setManufacturer("SonOff");
  SSDP.setManufacturerURL("https://github.com/vzakharchenko/smartthings-sonoff");
  SSDP.setDeviceType(ssdpDeviceType);
  SSDP.begin();
  Serial.println ( "SSDP server started" );
  int defaultState = storage.getDefaultState();
  bool lastState = storage.getLastState();

  Serial.println ( "storage.getDefaultState() = " + String(defaultState));
  Serial.println ( "configuration.lastState = " + String(lastState) );
  Serial.println("HTTPUpdateServer ready! Open http://" + String(IpAddress2String( WiFi.localIP())) + "/update in your browser");
  sonoff.setup();
  ArduinoOTA.begin();

  if (defaultState == 1) {
    switchOn(true);
  } else if (defaultState == 2) {
    if (lastState) {
      switchOn(true);
    } else {
      switchOff(true);
    }
  } else if (defaultState == 3) {
    int sthStatus = getSwitchState();
    if (sthStatus == 0) {
      switchOff(true);
    } else if (sthStatus == 1) {
      switchOn(true);
    } else {
      if (lastState) {
        switchOn(true);
      } else {
        switchOff(true);
      }
    }

  } else {
    switchOff(true);
  }
  ArduinoOTA.begin();
}

void loop ( void ) {
  ArduinoOTA.handle();
  yield();
  server.handleClient();
  sonoff.loop();
  boolean buttonState = sonoff.IsButtonOn();
  int deviceType = storage.getDeviceType();
  if (deviceType == SONOFF_INTERCOM) {
    if (sonoff.getSwitch()->getEvent() == SWITCH_EVENT_ON) {
      openDoor();
    }

    if (sonoff.IsButtonChanged()) {
      delay(50);
      smartThings.incomingCall(buttonState);
    }

  } else {
    if (sonoff.IsButtonChanged()) {
      if (buttonState) {
        switchOn(true);
      } else {
        switchOff(true);
      }
    }
    if (sonoff.getSwitch()->getEvent() == SWITCH_EVENT_ON) {
      if (sonoff.getRelay()->isOn()) {
        switchOff(true);
      }
      else {
        switchOn(true);
      }
    }
  }



}
