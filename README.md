# smartthings-sonoff
[![donate](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://secure.wayforpay.com/button/ba4989d5abdf7)  
integration smartthings and sonoff devices


# Supported Devices
SONOFF BASIC,
SONOFF POW,
SONOFF RF,
SONOFF TH,
SONOFF SV,
SLAMPHER,S20,
SONOFF TOUCH,
SONOFF POW R2,
SONOFF S31,
SONOFF T1 1CH,
ORVIBO B25,
SONOFF T1 1CH,
NODEMCU LOLIN,
D1 MINI RELAYSHIELD,
YJZK SWITCH 1CH,
WORKCHOICE ECOPLUG,
OPENENERGYMONITOR MQTT RELAY,
WION 50055,
EXS WIFI RELAY V31,
XENON SM PW702U,
ISELECTOR SM PW702,
ISELECTOR SM PW702U,
KMC 70011,
EUROMATE WIFI STECKER SCHUKO,
LINGAN SWA1

# Instalation Steps:
## flash device firmware:
  1. Hardware Preparation https://github.com/arendst/Sonoff-Tasmota/wiki/Hardware-Preparation
  2. download firmware from https://github.com/vzakharchenko/smartthings-sonoff/releases
  3. Flashing device: https://github.com/arendst/Sonoff-Tasmota/wiki/Flashing using firmware from step 2
  
## install custom DHT Source
  1. open https://graph.api.smartthings.com/location/list with your samsung account 
  2. select your location ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/Location.png?raw=true)
  3. open "My Device Handlers"  and click "create new Device Handler" ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/Device%20Handlers.png?raw=true)
  4. select "From Code" insert code from  https://raw.githubusercontent.com/vzakharchenko/smartthings-sonoff/master/smartthings/devices/Sonoff%20Switch.groovy ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/New%20Device%20Handler%20.png?raw=true)
  
  
## install SmartApp from Source
  1. open https://graph.api.smartthings.com/location/list with your samsung account
  2.. select your location  ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/Location.png?raw=true)
  3. add new smartapp  ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/addNewSmartApp.png?raw=true)
  4. select "From Code" insert code from https://raw.githubusercontent.com/vzakharchenko/smartthings-sonoff/master/smartthings/smartapps/SonOff%20devices.groovy
![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/New%20SmartApp.png?raw=true)
  
  5. open App-Setting ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/App%20Settings.png?raw=true)
  6. enable oauth in smartapp ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/enableOAuth.png?raw=true)
  7.  save and publish ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/saveAndPublish.png?raw=true)
  

## Configure in SmartThings Classic
  1. open SmartThings Classic
  2. open "Automation" Tab->"SmartApps" -> "add SmartApp" and select "My app" category.   ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/addSmartApp.png?raw=true)
  3. add "SonOff devices" ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/SonOffDevices.png?raw=true)
  4. wait until your devices will be available 
  5. select devices and click save ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/selectDevices.png?raw=true)


## Configure device
  1. open device ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/selectDevice.png?raw=true)
  2. configure device ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/configureDevice.png?raw=true)
  3. change name , select type or  power state at startup
 ![](https://github.com/vzakharchenko/smartthings-sonoff/blob/master/img/ConfigureDevicePage.png?raw=true)
  
##  Power states after booting device

  **Off** - start sonoff with relay off 
  
  **On** - start sonoff with relay on
  
  **Latest** - start sonoff with the latest state of ralay
  
  **SmartThings** - start sonoff with the  state from the SmartThings device
