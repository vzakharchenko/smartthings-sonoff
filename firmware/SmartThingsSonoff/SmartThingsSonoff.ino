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
#include "storage.h"
#include "Sonoff.h"
#include "SamsungSmartThings.h"
#include "index.h"

const char *ssid = "";
const char *password = "";

unsigned long previousMillis = 0;
const long interval = 2000;
#ifdef SWITCH14
boolean buttonStateLast = false;
#endif

ESP8266WebServer server ( 80 );
ESP8266HTTPUpdateServer httpUpdater;

Sonoff sonoff;
Storage storage;
SmartThings smartThings(&sonoff, &storage );
Ticker ticker;

const int led = LED;

void tick()
{
  //toggle state
  int state = digitalRead(led);  // get the current state of GPIO1 pin
  digitalWrite(led, !state);     // set pin to the opposite state
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
  sonoff.relay.on();
  digitalWrite ( led, 0 );
}



void relayOff() {
  sonoff.relay.off();
  digitalWrite ( led, 1 );
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

  if (force || !sonoff.relay.isOn()) {
    sw(true);
    int result = smartThings.on(force);
    boolean relayState =  result == 0 ? false : true;
    if (!force) {
      sw(relayState);
    }
  }

}

void switchOff(boolean force) {
  if (force || (sonoff.relay.isOn())) {
     sw(false);
    int result = smartThings.off(force);
    boolean relayState = result == 1 ? true : false;
    if (!force) {
      sw(relayState);
    }

  }
}

int getSwitchState() {
  return smartThings.getSmartThingsDevice().state;
}



void handleSettings () {
  String smartThingsUrl = server.arg("smartThingsUrl");
  String applicationId = server.arg("applicationId");
  String accessToken = server.arg("accessToken");
  String defaultStateString = server.arg("defaultState");
  storage.setSmartThingsUrl(smartThingsUrl);
  storage.setApplicationId(applicationId);
  storage.setAccessToken(accessToken);
  int defaultState = (defaultStateString == String("")) ? 0 : defaultStateString.toInt();
  storage.setDefaultState(defaultState);
  storage.save();
  smartThings.smartthingsInit();
  handleInfo();
}

void handleToggle () {
  if (sonoff.relay.isOn()) {
    switchOff(true);
  } else {
    switchOn(true);
  }
  handleInfo();
}

void handleOn () {
  switchOn(false);
  server.send ( 200, "application/json", "{ \"relay\": \"on\", \"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\",\"mac\":\"" + String(WiFi.macAddress()) + "\",\"pow\":\"" + String(sonoff.isPow()) + "\" }" );
}

void handleOff () {
  switchOff(false);
  server.send ( 200, "application/json", "{ \"relay\": \"off\", \"ip\":\"" + IpAddress2String( WiFi.localIP()) + "\" ,\"mac\":\"" + String(WiFi.macAddress()) + "\" ,\"pow\":\"" + String(sonoff.isPow()) + "\" }" );
}

void handleState () {
  SmartThingDevice std = smartThings.getSmartThingsDevice();
  String devStatus = "undefined";
  if (std.state == 0) {
    devStatus = "off";
  } else {
    devStatus = "on";
  }
  server.send ( 200, "application/json",
                "{ \"relay\": \""
                + String(sonoff.relay.isOn() ? "on" : "off")
                + "\",\"uptime\":" +
                String(millis()) +
                ", \"ssid\": \""
                + WiFi.SSID() +
                "\",\"ip\":\""
                + IpAddress2String( WiFi.localIP())
                + "\", \"mac\":\""
                + String(WiFi.macAddress())
                + "\", \"applicationId\":\""
                + String(storage.getApplicationId())
                + "\", \"accessToken\":\""
                + String(storage.getAccessToken())
                + "\", \"smartThingsUrl\":\""
                + String(storage.getSmartThingsUrl())
                + "\", \"smartthingsName\":\""
                + String(std.devName)
                + "\", \"versionFirmware\":\""
                + String(storage.getPackageVersion())+"."+String(storage.getStorageVersion())
                + "\", \"smartthingsStatus\":\""
                + devStatus
                + "\", \"pow\":\""
                + String(sonoff.isPow())
                + "\", \"defaultState\":"
                + String(storage.getDefaultState())
                + " }");
}

void handleInfo () {
  
  String payload = smartThings.getSmartThingsDevices();
  server.send ( 200, "application/json",
                "{ \"relay\": \""
                + String(sonoff.relay.isOn() ? "on" : "off")
                + "\",\"uptime\":" +
                String(millis()) +
                ", \"ssid\": \""
                + WiFi.SSID() +
                "\",\"ip\":\""
                + IpAddress2String( WiFi.localIP())
                + "\", \"mac\":\""
                + String(WiFi.macAddress())
                + "\", \"applicationId\":\""
                + String(storage.getApplicationId())
                + "\", \"accessToken\":\""
                + String(storage.getAccessToken())
                + "\", \"smartThingsUrl\":\""
                + String(storage.getSmartThingsUrl())
                + "\", \"smartthings\":"
                + String(payload)
                + ", \"versionFirmware\":\""
                + String(storage.getPackageVersion())+"."+String(storage.getStorageVersion())
                + "\", \"pow\":\""
                + String(sonoff.isPow())
                + "\", \"defaultState\":"
                + String(storage.getDefaultState())
                + " }");
}

void handleinit () {
  smartThings.smartthingsInit();
  handleInfo ();
}

void handlePow () {
  if (sonoff.isPow()) {
    server.send ( 200, "application/json",
                  "{ \"relay\": \""
                  + String(sonoff.relay.isOn() ? "on" : "off")
                  + "\",\"ip\":\""
                  + IpAddress2String( WiFi.localIP())
                  + "\", \"mac\":\""
                  + String(WiFi.macAddress())
                  + "\", \"pow\":\""
                  + String(sonoff.isPow())
                  + "\", \"voltage\":\""
                  + String(sonoff.getVoltage())
                  + "\", \"current\":\""
                  + String(sonoff.getCurrent())
                  + "\", \"activePower\":\""
                  + String(sonoff.getActivePower())
                  + "\", \"apparentPower\":\""
                  + String(sonoff.getApparentPower())
                  + "\", \"powerFactor\":\""
                  + String(sonoff.getPowerFactor())
                  + "\", \"reactivePower\":\""
                  + String(sonoff.getReactivePower())
                  + "\", \"energy\":\""
                  + String(sonoff.getEnergy())
                  + "\", \"currentMultiplier\":\""
                  + String(sonoff.getCurrentMultiplier())
                  + "\", \"voltageMultiplier\":\""
                  + String(sonoff.getVoltageMultiplier())
                  + "\", \"powerMultiplier\":\""
                  + String(sonoff.getPowerMultiplier())
                  + "\" }");
  } else {
    handleInfo ();
  }
}

void handleRoot() {
  server.send ( 200, "text/html", (const char *)target_index0_html );
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
  pinMode ( led, OUTPUT );
  Serial.begin ( 9600 );
  storage.load();
  ticker.attach(0.6, tick);
  WiFiManager wifiManager;
 // wifiManager.resetSettings();//todo
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect(ssid, password)) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }
  ticker.detach();
  digitalWrite ( led, 1 );

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
  server.on ( "/index.html", handleRoot );
  server.on ( "/config", HTTP_POST, handleSettings );
  server.on ( "/toggle", HTTP_POST, handleToggle );
  server.on ( "/on", handleOn);
  server.on ( "/off", handleOff );
  server.on ( "/state", handleState);
  server.on ( "/info", handleInfo);
  server.on ( "/health", handleinit);
  server.on ( "/pow", handlePow);
  server.onNotFound ( handleNotFound );
  httpUpdater.setup(&server);
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println ( "HTTP server started" );
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
#ifdef SWITCH14
  if (buttonStateLast != buttonState ) {
    if (buttonState) {
      switchOn(true);
    } else {
      switchOff(true);
    }
    buttonStateLast =  buttonState;
  }
#endif

  if (sonoff.sw.getEvent() == SWITCH_EVENT_ON) {
    if (sonoff.relay.isOn()) {
      switchOff(true);
    }
    else {
      switchOn(true);
    }
  }

}
