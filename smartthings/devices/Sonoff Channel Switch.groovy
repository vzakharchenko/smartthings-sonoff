metadata {
    definition(name: "Sonoff Channel Switch", namespace: "vzakharchenko", author: "Vasiliy Zakharchenko", vid: "generic-switch") {
        capability "Actuator"
        capability "Sensor"
        capability "Switch"
        capability "Health Check"
        attribute "ip", "string"
        attribute "mac", "string"
        attribute "channel", "string"
        attribute "port", "string"
        command "setIp"
        command "setPort"
        command "setMac"
        command "setChannel"
        command "forceOn"
        command "forceOff"
        command "markDeviceOnline"
        command "markDeviceOffline"
    }

    preferences {
        section() {
            input "powerStateAtStartup", title: "PowerState at startup", "enum", multiple: false, required: true, options: ["Off", "On", "Latest", "SmartThings"]
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
    apiPost("/config", null,
    "&defaultState=${["Off", "On", "Latest", "SmartThings"].indexOf(powerStateAtStartup)}" +
                    "&ch=${device.currentValue("channel")}"
            , "application/x-www-form-urlencoded")
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

def setMac(mac) {
    sendEvent(name: "mac", value: mac)
}

def setChannel(channel) {
    sendEvent(name: "channel", value: channel)
}

def apiGet(path, query) {
    def url = "${device.currentValue("ip")}:${device.currentValue("port")}";
    log.debug "request:  ${url}/${path}?ch=${device.currentValue('channel')} query= ${query}"
    def result = new physicalgraph.device.HubAction(
            method: 'GET',
            path: "/${path}?ch=${device.currentValue('channel')}",
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


def debug(message) {
    def debug = false
    if (debug) {
        log.debug message
    }
}
