# smartthings-sonoff
integration smartthings and sonoff devices

# Instalation Steps:
## 1. device firmware:
  1.1 Hardware Preparation https://github.com/arendst/Sonoff-Tasmota/wiki/Hardware-Preparation
  1.2. download firmware from https://github.com/vzakharchenko/smartthings-sonoff/releases
  1.3. Flashing device: https://github.com/arendst/Sonoff-Tasmota/wiki/Flashing using firmware from step 2
## 2. install DHT Source
  2.1 open https://graph.api.smartthings.com/location/list with your samsung account
  2.2 select your location 
  2.3 open "My Device Handlers"
  2.4 click "create new Device Handler"
  2.5 select "From Code" insert code from  
  
## 3. install SmartApp Source
  3.1. open https://graph.api.smartthings.com/location/list with your samsung account
  3.2. select your location 
  3.3. add new smartapp 
  3.4. select "From Code" insert code from https://raw.githubusercontent.com/vzakharchenko/smartthings-sonoff/master/smartthings/smartapps/SonOff%20devices.groovy
  3.5. open App-Setting
  3.6. enable oauth in smartapp
  3.7.  save and publish
 ## 4. Configure in SmartThings Classic
  4.1 open SmartThings Classic
  4.2 open "Automation" Tab->"SmartApps" -> "add SmartApp"
  4.3 select "My app" category
  4.4 add "SonOff devices"
  4.5. wait until your devices will be available
  4.6 select devices
  4.7 click save
  4.8 check for a new device Sonoff(<MAC>)
## 5. Configure devices
  5.1 open device
  5.2 configure device
  5.3 change name
  5.4 select type : Sonoff switch or Sonoff Basic with remote switch.
  5.5 select power state at startup
  5.5.1 off - start sonoff with relay off 
    5.5.2 on - start sonoff with relay on
    5.5.3 Latest - start sonoff with the latest state of ralay
    5.5.4 SmartThings - start sonoff with the  state from the SmartThings device
