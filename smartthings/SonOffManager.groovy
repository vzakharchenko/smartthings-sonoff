definition(
        name: "SonOff devices",
        minHubCoreVersion: '000.021.00001',
        executeCommandsLocally: true,
        mnmn: "SmartThings",
        namespace: "smartthings",
        author: "Vasiliy Zakharchenko",
        description: "Create/Delete SonOff devices",
        category: "My Apps",
        iconUrl: "https://cdn.itead.cc/media/catalog/product/cache/1/thumbnail/160x160/9df78eab33525d08d6e5fb8d27136e95/s/o/sonoff_03.jpg",
        iconX2Url: "https://cdn.itead.cc/media/catalog/product/cache/1/thumbnail/160x160/9df78eab33525d08d6e5fb8d27136e95/s/o/sonoff_03.jpg")


preferences {
    page(name: "config")
}

def config() {
    dynamicPage(name: "config", title: "SonOff Manager", install: true, uninstall: true) {

        section("on this hub...") {
            input "theHub", "hub", multiple: false, required: true
        }

        if (theHub) {
            if (state.sonoffMacDevices == null) {
                state.sonoffMacDevices = [:];
            }
            if (state.sonoffDevicesTimes == null) {
                state.sonoffDevicesTimes = [:];
            }
            section("Info") {
                if (!app.id) {
                    paragraph "After click save, please, open this smartapp once again!"
                } else if (!state.accessToken) {
                    paragraph "Did you forget to enable OAuth in SmartApp IDE settings?"
                } else {
                    paragraph "Please set \"applicationId\": ${app.id}, \"accessToken\": ${state.accessToken} on devices"
                }
            }
            if (!state.sonoffMacDevices.isEmpty()) {
                section("Device List") {
                    def deviceOptions = [];
                    state.sonoffMacDevices.each { mac, ip ->
                        deviceOptions.push(mac);
                    }
                    debug("deviceOptions:$deviceOptions")
                    input "sonoffs", "enum", multiple: true, required: false, title: "Select Devices (${deviceOptions.size()} found)", options: deviceOptions
                }

            }

        }
    }
}

def installed() {
    createAccessToken()
    getToken()
    debug("applicationId: $app.id")
    debug("accessToken: $state.accessToken")
    initialize()
}

def updated() {
    initialize()
}

def initialize() {
    unsubscribe();
    if (sonoffs) {
        sonoffs.each {
            def mac = it;
            def sonoffDevice = searchDevicesType("Virtual Switch").find {
                return it.getDeviceNetworkId() == mac
            };
            if (sonoffDevice == null) {
                sonoffDevice = addChildDevice("smartthings", "Virtual Switch", mac, theHub.id, [label: "Sonoff(${mac})", name: "Sonoff(${mac})"])
            }

            subscribe(sonoffDevice, "switch.on", switchOnHandler)
            subscribe(sonoffDevice, "switch.off", switchOffHandler)
        }
    }
    // subscribe(location, null, locationHandler)
    runEvery5Minutes(healthCheck)
}


mappings {
    path("/init") {
        action:
        [
                POST: "init"
        ]
    }
    path("/on") {
        action:
        [
                POST: "on"
        ]
    }
    path("/off") {
        action:
        [
                POST: "off"
        ]
    }
    path("/current") {
        action:
        [
                POST: "curState"
        ]
    }
}

def init() {
    def json = request.JSON;
    state.sonoffMacDevices.put(json.mac, json.ip);
    def relay = json.relay;
    def sonoffDevice = searchDevicesType("Virtual Switch").find {
        return it.getDeviceNetworkId() == json.mac
    };
    if (sonoffDevice != null) {
        debug("init: ${json.ip}:${sonoffDevice.getDeviceNetworkId()}:${relay}");
        if (relay.equals("on")) {
            sonoffDevice.on();
        } else if (relay.equals("off")) {
            sonoffDevice.off();
        }
        updateActiveTime(json.mac)
    }

    debug("init: $json")
    return [status: "ok"]
}

def updateActiveTime(mac) {
    def date = new Date();
    debug("update time for ${mac} states: ${state.sonoffDevicesTimes}");
    state.sonoffDevicesTimes.put(mac, date.getTime());
}

def on() {
    def json = request.JSON;
    debug("on: $json")
    def relay = "undefined";
    state.sonoffMacDevices.put(json.mac, json.ip);
    def sonoffDevice = searchDevicesType("Virtual Switch").find {
        return it.getDeviceNetworkId() == json.mac
    };
    if (sonoffDevice != null) {
        if (json.force) {
            sonoffDevice.on();
            relay = "on";
        } else {
            def switchData = sonoffDevice.currentState("switch");
            def switchState = switchData.value;
            if (switchState == "on") {
                relay = "on";
            } else {
                relay = "off";
            }
        }

        updateActiveTime(json.mac)
    }


    return [relay: relay]
}

def off() {
    def json = request.JSON;
    state.sonoffMacDevices.put(json.mac, json.ip);
    def sonoffDevice = searchDevicesType("Virtual Switch").find {
        return it.getDeviceNetworkId() == json.mac
    };
    if (sonoffDevice != null) {
        sonoffDevice.off()
        updateActiveTime(json.mac)
    }

    debug("off: $json")
    return [status: "ok"]
}

def curState() {
    def state = "undefined";
    def name = "undefined";
    def json = request.JSON;
    // state.sonoffMacDevices.put(json.mac, json.ip);
    def sonoffDevice = searchDevicesType("Virtual Switch").find {
        return it.getDeviceNetworkId() == json.mac
    };
    if (sonoffDevice != null) {
        def switchData = sonoffDevice.currentState("switch");
        state = switchData.value
        name = switchData.linkText;
    }
    return [status: state, name: name]
}

def getToken() {
    if (!state.accessToken) {
        try {
            getAccessToken()
            DEBUG("Creating new Access Token: $state.accessToken")
        } catch (ex) {
            DEBUG("Did you forget to enable OAuth in SmartApp IDE settings")
            DEBUG(ex)
        }
    }
}

def searchDevicesType(devType) {
    def typeDevices = []
    childDevices.each {
        if (it.getTypeName() == devType) {
            typeDevices.add(it)
        }
    }
    return typeDevices
}

def switchOnHandler(evt) {
    def mac = evt.getDevice().getDeviceNetworkId();
    def ip = state.sonoffMacDevices.get(mac);
    runIn(1, "switchHandler", [data: [ip: ip, mode: "on"], overwrite: true])
}

def switchOffHandler(evt) {
    def mac = evt.getDevice().getDeviceNetworkId();
    def ip = state.sonoffMacDevices.get(mac);
    runIn(1, "switchHandler", [data: [ip: ip, mode: "off"], overwrite: true])
}

def switchHandler(data) {
    debug("switchHandler = ${data}")
    def ip = data.ip;
    apiGet(ip, 80, data.mode, [])
}

def healthCheck() {
    def devices = searchDevicesType("Virtual Switch")
    def timeout = 1000 * 60 * 30;
    def curTime = new Date().getTime();
    devices.each {
        def mac = it.getDeviceNetworkId();
        def ip = state.sonoffMacDevices.get(mac);
        it.parent.apiGet(ip, 80, "health", [])
        def activeDate = state.sonoffDevicesTimes.get(mac);
        if ((curTime - timeout) > activeDate) {
            it.setOffline();
            debug("ip ${ip} offline ${curTime - timeout} > ${activeDate} ")
        } else {
            debug("ip ${ip} online ${curTime - timeout} < ${activeDate} ")
        }
    }
}
/*
def locationHandler(evt) {
    def description = evt.description
    def msg = parseLanMessage(description)
    def json = msg.json
    def ip = json.ip;
    def relay = json.relay;
    debug("ip $ip : $relay");
    def sonoffDevice = searchDevicesType("Virtual Switch").find {
        return it.getDeviceNetworkId() == json.mac
    };
    if (sonoffDevice != null) {
        updateActiveTime(json.mac)
        if (relay.equals("on")) {

            sonoffDevice.on();
        } else if (relay.equals("off")) {
            sonoffDevice.off();
        }
    }
}*/

def apiGet(ip, port, path, query) {
    def url = "${ip}:${port}";
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

def debug(message) {
    def debug = false
    if (debug) {
        log.debug message
    }
}
