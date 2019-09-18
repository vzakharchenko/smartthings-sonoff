metadata {
    definition(name: "Sonoff MultiChannel", namespace: "vzakharchenko", author: "Vasiliy Zakharchenko") {
        capability "Actuator"
        capability "Sensor"
        capability "Health Check"
        attribute "ip", "string"
        attribute "port", "string"
        attribute "channels", "string"

        command "setIp"
        command "setPort"
        command "setChannels"
        command "markDeviceOnline"
        command "markDeviceOffline"
        command "subscribeCommand"
    }

    preferences {
        def supportedDevices = [
                "SONOFF DUAL",
                "SONOFF DUAL R2",
                "SONOFF T1 2CH",
                "ELECTRODRAGON WIFI IOT",
                "YJZK SWITCH 2CH",
                "SONOFF T1 3CH",
                "YJZK SWITCH 3CH",
                "SONOFF 4CH",
                "SONOFF 4CH PRO",
                "SONOFF IFAN02"
        ];
        section() {
            input "deviceType", "enum", title: "Device Type", multiple: false, required: true, options: supportedDevices
        }
    }

    tiles(scale: 2) {
        valueTile("health", "device.healthStatus", decoration: "flat", width: 2, height: 2) {
            state "healthStatus", label: '${currentValue}'
        }
        // the "switch" tile will appear in the Things view
        main("health")

        // the "switch" and "power" tiles will appear in the Device Details
        // view (order is left-to-right, top-to-bottom)
        details(["health"])
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
            "SONOFF DUAL",
            "SONOFF DUAL R2",
            "SONOFF T1 2CH",
            "ELECTRODRAGON WIFI IOT",
            "YJZK SWITCH 2CH",
            "SONOFF T1 3CH",
            "YJZK SWITCH 3CH",
            "SONOFF 4CH",
            "SONOFF 4CH PRO",
            "SONOFF IFAN02"
    ]
    ;
    debug("getCallBackAddress() = ${getCallBackAddress()}");
    apiPost("/config", null,
            "&deviceType=${supportedDevices.indexOf(deviceType)}" +
                    "&callback=${getCallBackAddress()}" +
                    "&hub_host=${device.hub.getDataValue("localIP")}" +
                    "&hub_port=${device.hub.getDataValue("localSrvPortTCP")}"
            , "application/x-www-form-urlencoded")
    runIn(5, subscribeHandler);
    runEvery1Hour(subscribeHandler);
}

def subscribeHandler() {
    apiPost("/config", null,
            "hub_host=${device.hub.getDataValue("localIP")}" +
                    "&hub_port=${device.hub.getDataValue("localSrvPortTCP")}"
            , "application/x-www-form-urlencoded")
    subscribeAction("/subscribe");
}

def subscribeCommand() {
    subscribeHandler();
}

def parse(description) {
    def msg = parseLanMessage(description);
    def bodyString = msg.body
    if (bodyString) {
        if (bodyString) {
            def json = msg.json;
            if (json) {

                debug("parent: ${parent}")
                debug("device.getDeviceNetworkId() = ${device.getDeviceNetworkId()}")
                debug("Values received: ${json}")
                parent.fromMultiChannel(device.getDeviceNetworkId(), json);
                if (json.action != null) {

                    if (json.action == "subscribe") {
                        subscribeCommand();
                    }
                }


            }
        }
    }
}

def installed() {
}

def setIp(ip) {
    sendEvent(name: "ip", value: ip)
}


def setPort(ip) {
    sendEvent(name: "port", value: ip)
}

def setChannels(channels) {
    sendEvent(name: "channels", value: channels)
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

private String getCallBackAddress() {
    return "http://" + device.hub.getDataValue("localIP") + ":" + device.hub.getDataValue("localSrvPortTCP") + "/notify"
}

private subscribeAction(path) {
    def url = "${device.currentValue("ip")}:${device.currentValue("port")}";
    log.trace "subscribe($path, $callbackPath)"
    def address = getCallBackAddress()

    def result = new physicalgraph.device.HubAction(
            method: "SUBSCRIBE",
            path: path,
            headers: [
                    HOST    : url,
                    CALLBACK: "<${address}>",
                    NT      : "upnp:event",
                    TIMEOUT : "Second-28800"
            ]
    )

    log.trace "SUBSCRIBE $path"

    return result
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
