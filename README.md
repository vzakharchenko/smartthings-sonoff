# smartthings-sonoff
integration smartthings and sonoff devices

# Instalation Steps:
## flash device firmware:
  1. Hardware Preparation https://github.com/arendst/Sonoff-Tasmota/wiki/Hardware-Preparation
  2. download firmware from https://github.com/vzakharchenko/smartthings-sonoff/releases
  3. Flashing device: https://github.com/arendst/Sonoff-Tasmota/wiki/Flashing using firmware from step 2
  
## install custom DHT Source
  1. open https://graph.api.smartthings.com/location/list with your samsung account 
  2. select your location <Location.png>
  3. open "My Device Handlers"  and click "create new Device Handler" <Device Handlers.png>
  4. select "From Code" insert code from  https://raw.githubusercontent.com/vzakharchenko/smartthings-sonoff/master/smartthings/devices/Sonoff%20Switch.groovy <New Device Handler .png>
  
  
## install SmartApp from Source
  1. open https://graph.api.smartthings.com/location/list with your samsung account
  2.. select your location <Location.png>
  3. add new smartapp  <addNewSmartApp.png>
  4. select "From Code" insert code from https://raw.githubusercontent.com/vzakharchenko/smartthings-sonoff/master/smartthings/smartapps/SonOff%20devices.groovy
  <New SmartApp.png>
  
  5. open App-Setting <App Settings.png>
  6. enable oauth in smartapp <enableOAuth.png>
  7.  save and publish <saveAndPublish.png>
  

## Configure in SmartThings Classic
  1. open SmartThings Classic
  2. open "Automation" Tab->"SmartApps" -> "add SmartApp" and select "My app" category.   <addSmartApp.png>
  3. add "SonOff devices" <SonOffDevices.png>
  4. wait until your devices will be available <SonOffDevices.png>
  5. select devices and click save <selectDevices.png>
  6. check for a new device Sonoff(<MAC>)

## Configure device
  1. open device <selectDevice.png>
  2. configure device <configureDevice.png>
  3. change name , select type or  power state at startup
  <ConfigureDevicePage.png>
  
##  Power states after booting device
  **Off** - start sonoff with relay off 
  **On** - start sonoff with relay on
  **Latest** - start sonoff with the latest state of ralay
  **SmartThings** - start sonoff with the  state from the SmartThings device
