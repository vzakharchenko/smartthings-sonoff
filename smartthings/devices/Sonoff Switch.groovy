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
        section() {
            paragraph "Device Type"
            input "deviceType", "enum", title: "Device Type", multiple: false, required: false, options: ["Sonoff Switch", "Sonoff Basic (Remote GPIO14"], defaultValue: "Sonoff Switch"
        }
        section() {
            input "powerStateAtStartup", title: "PowerState at startup", "enum", multiple: false, required: false, options: ["Off", "On", "Latest", "SmartThings"], defaultValue: "Off"
        }
        if (deviceType.equals("Sonoff Basic (Remote GPIO14)")) {
            section() {
                paragraph "Default Switch Value"
                input "gpio14State", title: "Default Switch Value", "enum", multiple: false, required: false, options: ["HIGH", "LOW"], defaultValue: "HIGH";
            }
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
    apiPost("/config", null,
            "&defaultState=${["Off", "On", "Latest", "SmartThings"].indexOf(powerStateAtStartup)}" +
                    "&deviceType=${["Sonoff Switch", "Intercom", "Sonoff Basic (Remote GPIO14"].indexOf(deviceType)}" +
                    "&gpio14State=${["HIGH", "LOW"].indexOf(gpio14State0)}"
            , "application/x-www-form-urlencoded")
}

def parse(description) {
    debug("description=${description}");
}

def on() {
    apiGet("on", null);
}

def off() {
    apiGet("off", null);
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
    setDeviceHealth("online")
}

def markDeviceOffline() {
    sendEvent(name: "switch", value: "offline", descriptionText: "The device is offline")
    setDeviceHealth("offline")

}

private setDeviceHealth(String healthState) {
    debug("healthStatus: ${device.currentValue('healthStatus')}; DeviceWatch-DeviceStatus: ${device.currentValue('DeviceWatch-DeviceStatus')}")
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
    def debug = true
    if (debug) {
        log.debug message
    }
}
