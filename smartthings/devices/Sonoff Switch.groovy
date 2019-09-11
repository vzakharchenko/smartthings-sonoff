metadata {
    definition(name: "Sonoff Switch", namespace: "vzakharchenko", author: "Vasiliy Zakharchenko", vid: "generic-switch") {
        capability "Actuator"
        capability "Sensor"
        capability "Switch"
        capability "Health Check"
        attribute "ip", "string"
        attribute "port", "string"

        command "setIp"
        command "setPort"
        command "forceOn"
        command "forceOff"
        command "markDeviceOnline"
        command "markDeviceOffline"
    }

    preferences {
        def supportedDevices = [
                "SONOFF BASIC",
                "SONOFF POW",
                "SONOFF RF",
                "SONOFF TH",
                "SONOFF SV",
                "SLAMPHER",
                "S20",
                "SONOFF TOUCH",
                "SONOFF POW R2",
                "SONOFF S31",
                "SONOFF T1 1CH",
                "ORVIBO B25",
                "SONOFF T1 1CH",
                "NODEMCU LOLIN",
                "D1 MINI RELAYSHIELD",
                "YJZK SWITCH 1CH",
                "WORKCHOICE ECOPLUG",
                "OPENENERGYMONITOR MQTT RELAY",
                "WION 50055",
                "EXS WIFI RELAY V31",
                "XENON SM PW702U",
                "ISELECTOR SM PW702",
                "ISELECTOR SM PW702U",
                "KMC 70011",
                "EUROMATE WIFI STECKER SCHUKO",
                "LINGAN SWA1"
        ];
        section() {
            input "deviceType", "enum", title: "Device Type", multiple: false, required: true, options: supportedDevices
        }
        section() {
            input "powerStateAtStartup", title: "PowerState at startup", "enum", multiple: false, required: true, options: ["Off", "On", "Latest", "SmartThings"]
        }
        section() {
            input "externalSwitchPin", title: "External Switch GPIO Pin", "enum", multiple: false, required: false, options: [16, 5, 4, 0, 2, 14, 12, 13, 15];
        }
        section() {
            input "gpio14State", title: "External Switch Value", "enum", multiple: false, required: false, options: ["HIGH", "LOW"], defaultValue: "HIGH";
        }
    }

    tiles(scale: 2) {
        multiAttributeTile(name: "switch", type: "lighting", width: 6, height: 4, canChangeIcon: true) {
            tileAttribute("device.switch", key: "PRIMARY_CONTROL") {
                attributeState "on", label: '${name}', action: "switch.off", icon: "st.Home.home30", backgroundColor: "#00A0DC", nextState: "turningOff"
                attributeState "off", label: '${name}', action: "switch.on", icon: "st.Home.home30", backgroundColor: "#FFFFFF", nextState: "turningOn", defaultState: true
                attributeState "turningOn", label: 'Turning On', action: "switch.off", icon: "st.Home.home30", backgroundColor: "#00A0DC", nextState: "turningOn"
                attributeState "turningOff", label: 'Turning Off', action: "switch.on", icon: "st.Home.home30", backgroundColor: "#FFFFFF", nextState: "turningOff"
                attributeState "offline", label: '${name}', icon: "st.switches.switch.off", backgroundColor: "#cccccc"
            }
        }

        main(["switch"])
        details(["switch"])

    }
}

def updated() {
    def gpio14State0 = gpio14State ? gpio14State : "HIGH";
    def supportedDevices = [
            "SONOFF BASIC",
            "SONOFF POW",
            "SONOFF RF",
            "SONOFF TH",
            "SONOFF SV",
            "SLAMPHER",
            "S20",
            "SONOFF TOUCH",
            "SONOFF POW R2",
            "SONOFF S31",
            "SONOFF T1 1CH",
            "ORVIBO B25",
            "SONOFF T1 1CH",
            "NODEMCU LOLIN",
            "D1 MINI RELAYSHIELD",
            "YJZK SWITCH 1CH",
            "WORKCHOICE ECOPLUG",
            "OPENENERGYMONITOR MQTT RELAY",
            "WION 50055",
            "EXS WIFI RELAY V31",
            "XENON SM PW702U",
            "ISELECTOR SM PW702",
            "ISELECTOR SM PW702U",
            "KMC 70011",
            "EUROMATE WIFI STECKER SCHUKO",
            "LINGAN SWA1"
    ]
    ;
    debug("getCallBackAddress() = ${getCallBackAddress()}");
    apiPost("/config", null,
            "&defaultState=${["Off", "On", "Latest", "SmartThings"].indexOf(powerStateAtStartup)}" +
                    "&deviceType=${supportedDevices.indexOf(deviceType)}" +
                    "&externalSwitchState=${["HIGH", "LOW"].indexOf(gpio14State0)}" +
                    "&externalSwitchPin=${externalSwitchPin == null ? -1 : externalSwitchPin}" +
                    "&callback=${getCallback()}" +
                    "&hub_host=${device.hub.getDataValue("localIP")}"+
                    "&hub_port=${device.hub.getDataValue("localSrvPortTCP")}"
            , "application/x-www-form-urlencoded")
    subscribeHandler();
    runEvery1Hour(subscribeHandler);
}

def subscribeHandler(){
    subscribeAction("/subscribe");
}

def parse(description) {
    def msg = parseLanMessage(description);
    def bodyString = msg.body
    if (bodyString) {
        def json = msg.json;
        if (json) {
            debug("Values received: ${json}")
            if (json.relay == "on") {
                forceOn();
            } else if (json.relay == "off") {
                forceOff();
            }
            if (json.ip) {
                setIp(json.ip);
            }
        }
    }
}

def on() {
    apiGet("on", null);
}

def off() {
    if (device.currentValue('switch') == "offline") {
        forceOff();
    } else {
        apiGet("off", null);
    }
}


def forceOn() {
    sendEvent(name: "switch", value: "on")
}

def forceOff() {
    sendEvent(name: "switch", value: "off")
}

def installed() {
}

def setIp(ip) {
    sendEvent(name: "ip", value: ip)
}


def setPort(ip) {
    sendEvent(name: "port", value: ip)
}

def apiGet(path, query) {
    def url = "${device.currentValue("ip")}:${device.currentValue("port")}";
    log.debug "request:  ${url}/${path} query= ${query}"
    def result = new physicalgraph.device.HubAction(
            method: 'GET',
            path: "/${path}",
            headers: [
                    HOST  : url,
                    Accept: "*/*",
                    test  : "testData"
            ],
            query: query
    )

    return sendHubCommand(result)
}

def markDeviceOnline() {
    debug("switchStatus: ${device.currentValue('switch')};")
    if (device.currentValue('switch') == "offline") {
        apiGet("info", null);
    }
    setDeviceHealth("online")
}

def markDeviceOffline() {
    sendEvent(name: "switch", value: "offline", descriptionText: "The device is offline")
    setDeviceHealth("offline")

}

private setDeviceHealth(String healthState) {
    //debug("healthStatus: ${device.currentValue('healthStatus')}; DeviceWatch-DeviceStatus: ${device.currentValue('DeviceWatch-DeviceStatus')}")
    // ensure healthState is valid
    List validHealthStates = ["online", "offline"]
    healthState = validHealthStates.contains(healthState) ? healthState : device.currentValue("healthStatus")
    // set the healthState
    sendEvent(name: "DeviceWatch-DeviceStatus", value: healthState)
    sendEvent(name: "healthStatus", value: healthState)
}

def apiPost(path, query, body, contentType) {
    def url = "${device.currentValue("ip")}:${device.currentValue("port")}";
    log.debug "request:  ${url}${path} query= ${query} body=${body} contentType=${contentType} "
    def result = new physicalgraph.device.HubAction(
            method: 'POST',
            path: path,
            headers: [
                    HOST          : url,
                    Accept        : "*/*",
                    "Content-Type": contentType
            ],
            body: body
    )

    return sendHubCommand(result)
}

private String getCallBackAddress() {
    return "http://"+device.hub.getDataValue("localIP") + ":" + device.hub.getDataValue("localSrvPortTCP")+"/notify"
}

private subscribeAction(path) {
    def url = "${device.currentValue("ip")}:${device.currentValue("port")}";
    log.trace "subscribe($path, $callbackPath)"
    def address = getCallBackAddress()

    def result = new physicalgraph.device.HubAction(
            method: "SUBSCRIBE",
            path: path,
            headers: [
                    HOST: url,
                    CALLBACK: "<${address}>",
                    NT: "upnp:event",
                    TIMEOUT: "Second-28800"
            ]
    )

    log.trace "SUBSCRIBE $path"

    return result
}

def debug(message) {
    def debug = false
    if (debug) {
        log.debug message
    }
}
