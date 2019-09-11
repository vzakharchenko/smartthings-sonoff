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




const char *ssid = "";
const char *password = "";

const String ssdpDeviceType = SSDP_DEVICE_TYPE;

unsigned long previousMillis = 0;
const long interval = 2000;

int nexSeq = 0;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
SSDPClass2 SSDP2;


Storage storage;
Sonoff sonoff(&storage);
SmartThings smartThings(&sonoff, &storage );
DeviceHandler deviceHandler(&smartThings);
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
  digitalWrite ( sonoff.getLed(), storage.getLedInverse() == 0 ? 1 : 0 );
}



void relayOff() {
  sonoff.getRelay()->off();
  digitalWrite ( sonoff.getLed(), storage.getLedInverse() == 0 ? 0 : 1 );
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
  return smartThings.getSwitchState();
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
    Device device(value);
    storage.setRelayPin(device.getRelayPin());
    storage.setSwitchPin(device.getSwitchPin());
    storage.setLedPin(device.getLedPin());
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
  handleInfo();
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
  switchOn(false);
  server.send ( 200, "application/json", "{ \"relay\": \"on\", \"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\" }" );
}


void handleOff () {
  switchOff(false);
  server.send ( 200, "application/json", "{ \"relay\": \"off\", \"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\" ,\"mac\":\"" + String(WiFi.macAddress()) + "\"  }" );
}

void handleInfo () {
  String payload = smartThings.getSmartThingsDevice();
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
                + "\",\"supportedDevices\":"
                + getDeviceJson()
                + ", \"versionFirmware\":\""
                + String(storage.getPackageVersion()) + "." + String(storage.getStorageVersion())
                + "\", \"relayPin\":"
                + String(storage.getRelayPin())
                + ", \"switchPin\":"
                + String(storage.getSwitchPin())
                + ", \"externalSwitchPin\":"
                + String(storage.getExternalSwitchPin())
                + ", \"ledPin\":"
                + String(storage.getLedPin())
                + ", \"externalSwitchState\":"
                + String(storage.getExternalSwitchState())
                + ", \"defaultState\":"
                + String(storage.getDefaultState())
                + " }");
  smartThings.smartthingsInit();
}

void handleRoot() {
  String payload = smartThings.getSmartThingsDevice();
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
  storage.save();
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
  server.on ( "/off", handleOff );
  server.on ( "/info", handleInfo);
  server.on("/description.xml", handleDescription);
  server.on ( "/subscribe", handleSubscription );
  server.onNotFound ( handleNotFound );
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
  int defaultState = storage.getDefaultState();
  bool lastState = storage.getLastState();

  Serial.println ( "storage.getDefaultState() = " + String(defaultState));
  Serial.println ( "configuration.lastState = " + String(lastState) );
  Serial.println("HTTPUpdateServer ready! Open http://" + String(IpAddress2String( WiFi.localIP())) + "/update in your browser");
  sonoff.setup();
  smartThings.subscribe();

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

  int seq = storage.getSeq();
  storage.setSeq(seq + 1);
  deviceHandler.begin();
}

void loop ( void ) {

  server.handleClient();
  yield();
  sonoff.loop();
  yield();
  deviceHandler.loop();
  boolean buttonState = sonoff.IsButtonOn();
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
