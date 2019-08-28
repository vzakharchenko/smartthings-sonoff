definition(
        name: "SonOff devices",
        minHubCoreVersion: '000.021.00001',
        executeCommandsLocally: true,
        namespace: "vzakharchenko",
        author: "Vasiliy Zakharchenko",
        description: "Create/Delete SonOff devices",
        category: "My Apps",
        iconUrl: "https://cdn-media.itead.cc/media/catalog/product/s/o/sonoff_03.jpg",
        iconX2Url: "https://cdn-media.itead.cc/media/catalog/product/s/o/sonoff_03.jpg")


preferences {
    page(name: "config", refreshInterval: 5)
}

def config() {
    if (!state.clearDevices) {
        state.sonoffMacDevices = [:];
        state.clearDevices = true
    }
    if (!state.subscribe) {
        subscribe(location, "ssdpTerm." + getURN(), locationHandler)
        state.subscribe = true
    }
    if (state.sonoffMacDevices == null) {
        state.sonoffMacDevices = [:];
    }
    int refreshCount = !state.refreshCount ? 0 : state.refreshCount as int
    state.refreshCount = refreshCount + 1
    def refreshInterval = refreshCount == 0 ? 2 : 5
    //ssdp request every fifth refresh
    if ((refreshCount % 2) == 0) {
        ssdpDiscover();
    }

    if (!state.deviceTypes) {
        state.deviceTypes = ["Sonoff Switch", "Sonoff Switch (Remote GPIO14)"];
    }
    if (!state.powerStateAtStartupList) {
        state.powerStateAtStartupList = ["Off", "On", "Latest", "SmartThings"];
    }
    if (!state.gpio14StateList) {
        state.gpio14StateList = ["HIGH", "LOW"];
    }

    dynamicPage(name: "config", title: "SonOff Manager", refreshInterval: refreshInterval, install: true, uninstall: true) {
        section("on this hub...") {
            input "theHub", "hub", multiple: false, required: true, defaultValue: state.hub
        }

        if (state.sonoffMacDevices == null) {
            state.sonoffMacDevices = [:];
        }
        if (state.sonoffDevicesTimes == null) {
            state.sonoffDevicesTimes = [:];
        }
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
    state.clearDevices = false
    state.subscribe = false;
    if (sonoffs) {
        sonoffs.each {
            def mac = it;

            def ip = state.sonoffMacDevices.get(mac);
            debug("mac = ${mac}");
            def sonoffDevice = searchDevicesType("Sonoff Switch").find {
                return it.getDeviceNetworkId() == mac
            };
            if (sonoffDevice == null) {
                sonoffDevice = addChildDevice("vzakharchenko", "Sonoff Switch", mac, theHub.id, [label: "Sonoff(${mac})", name: "Sonoff(${mac})"])
            }
            sonoffDevice.setIp(ip);
            sonoffDevice.setPort("80");
            apiPost(ip, 80, "/config", null,
                    "applicationId=${app.id}" +
                            "&accessToken=${state.accessToken}"
                    , "application/x-www-form-urlencoded")
        }
    }
    if (!state.subscribe) {
        subscribe(location, "ssdpTerm." + getURN(), locationHandler)
        state.subscribe = true
    }
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

    path("/info") {
        action:
        [
                POST: "info"
        ]
    }
}

def init() {
    def json = request.JSON;
    def mac = modifyMac(json.mac);
    state.sonoffMacDevices.put(mac, json.ip);
    def relay = json.relay;
    def sonoffDevice = searchDevicesType("Sonoff Switch").find {
        return it.getDeviceNetworkId() == mac
    };
    if (sonoffDevice != null) {
        debug("init: ${json.ip}:${sonoffDevice.getDeviceNetworkId()}:${relay}");
        if (relay.equals("on")) {
            sonoffDevice.forceOn();
        } else if (relay.equals("off")) {
            sonoffDevice.forceOff();
        }
        sonoffDevice.markDeviceOnline();
        updateActiveTime(mac)
    }

    debug("init: $json")
    return "OK"
}

def updateActiveTime(mac) {
    def date = new Date();
    debug("update time for ${mac} states: ${state.sonoffDevicesTimes}");
    def sonoffDevice = searchDevicesType("Sonoff Switch").find {
        return it.getDeviceNetworkId() == mac
    };
    if (sonoffDevice != null) {
        sonoffDevice.markDeviceOnline();
    }
    state.sonoffDevicesTimes.put(mac, date.getTime());
}

def on() {
    def json = request.JSON;
    def mac = modifyMac(json.mac);
    debug("on: $json")
    state.sonoffMacDevices.put(mac, json.ip);
    def sonoffDevice = searchDevicesType("Sonoff Switch").find {
        return it.getDeviceNetworkId() == mac
    };
    if (sonoffDevice) {
        sonoffDevice.markDeviceOnline();
        sonoffDevice.forceOn();
        updateActiveTime(mac);

    }

    debug("on: relay: $relay")
    return [relay: relay]
}

def off() {
    def json = request.JSON;
    def mac = modifyMac(json.mac);
    state.sonoffMacDevices.put(mac, json.ip);
    def sonoffDevice = searchDevicesType("Sonoff Switch").find {
        return it.getDeviceNetworkId() == mac
    };
    if (sonoffDevice != null) {
        sonoffDevice.markDeviceOnline();
        sonoffDevice.forceOff()
        updateActiveTime(mac);

    }

    debug("off: $json")
    return [status: "ok"]
}

def curState() {
    def state = "undefined";
    def name = "undefined";
    def deviceName = "undefined";
    def json = request.JSON;
    def mac = modifyMac(json.mac);
    debug("curState ${json}");
    // state.sonoffMacDevices.put(json.mac, json.ip);
    def sonoffDevice = searchDevicesType("Sonoff Switch").find {
        return it.getDeviceNetworkId() == mac
    };
    if (sonoffDevice != null) {
        def switchData = sonoffDevice.currentState("switch");
        if (switchData) {
            state = switchData.value
            name = switchData.linkText;
            updateActiveTime(mac)
        }
        deviceName = sonoffDevice.getLabel();
    }
    return [status: state, name: name, deviceName: deviceName]
}

def info() {
    def curState = curState();
    return curState
}

def getURN() {
    return "urn:sonoff:device:vzakharchenko:1"
}

void ssdpDiscover() {
    debug("send lan discovery " + getURN())
    sendHubCommand(new physicalgraph.device.HubAction("lan discovery " + getURN(), physicalgraph.device.Protocol.LAN))
}

def locationHandler(evt) {
    def description = evt?.description
    debug("event: ${description}");
    def urn = getURN()
    def hub = evt?.hubId
    def parsedEvent = parseEventMessage(description)
    state.hub = hub
    if (parsedEvent?.ssdpTerm?.contains(urn)) {
        if (state.sonoffMacDevices == null) {
            state.sonoffMacDevices = [:];
        }
        def ip = convertHexToIP(parsedEvent.ip)
        state.sonoffMacDevices.put(modifyMac(parsedEvent.mac), ip);
    }

}

def checkSonOff(parsedEvent) {
    def timeout = 1000 * 60 * 30;
    def curTime = new Date().getTime();

    def devices = searchDevicesType("Sonoff Switch");
    def device = devices.find {
        return it.getDeviceNetworkId() == modifyMac(parsedEvent.mac)
    };
    if (device) {
        device.setIp(parsedEvent.ip);
        device.setPort(parsedEvent.port);
        state.sonoffDevicesTimes.put(modifyMac(parsedEvent.mac), curTime)
    }


    devices.each {
        def mac = it.getDeviceNetworkId();
        def ip = state.sonoffMacDevices.get(mac);
        def activeDate = state.sonoffDevicesTimes.get(mac);
        if ((curTime - timeout) > activeDate) {
            it.setOffline();
            it.markDeviceOffline();
            debug("ip ${ip} offline ${curTime - timeout} > ${activeDate} ")
        } else {
            it.markDeviceOnline();
            debug("ip ${ip} online ${curTime - timeout} < ${activeDate} ")
        }
    }
}

def String convertHexToIP(hex) {
    [convertHexToInt(hex[0..1]), convertHexToInt(hex[2..3]), convertHexToInt(hex[4..5]), convertHexToInt(hex[6..7])].join(".")
}

def Integer convertHexToInt(hex) {
    Integer.parseInt(hex, 16)
}

private def parseEventMessage(String description) {
    def event = [:]
    def parts = description.split(',')

    parts.each
            { part ->
                part = part.trim()
                if (part.startsWith('devicetype:')) {
                    def valueString = part.split(":")[1].trim()
                    event.devicetype = valueString
                } else if (part.startsWith('mac:')) {
                    def valueString = part.split(":")[1].trim()
                    if (valueString) {
                        event.mac = valueString
                    }
                } else if (part.startsWith('networkAddress:')) {
                    def valueString = part.split(":")[1].trim()
                    if (valueString) {
                        event.ip = valueString
                    }
                } else if (part.startsWith('deviceAddress:')) {
                    def valueString = part.split(":")[1].trim()
                    if (valueString) {
                        event.port = valueString
                    }
                } else if (part.startsWith('ssdpPath:')) {
                    def valueString = part.split(":")[1].trim()
                    if (valueString) {
                        event.ssdpPath = valueString
                    }
                } else if (part.startsWith('ssdpUSN:')) {
                    part -= "ssdpUSN:"
                    def valueString = part.trim()
                    if (valueString) {
                        event.ssdpUSN = valueString

                        def uuid = getUUIDFromUSN(valueString)

                        if (uuid) {
                            event.uuid = uuid
                        }
                    }
                } else if (part.startsWith('ssdpTerm:')) {
                    part -= "ssdpTerm:"
                    def valueString = part.trim()
                    if (valueString) {
                        event.ssdpTerm = valueString
                    }
                } else if (part.startsWith('headers')) {
                    part -= "headers:"
                    def valueString = part.trim()
                    if (valueString) {
                        event.headers = valueString
                    }
                } else if (part.startsWith('body')) {
                    part -= "body:"
                    def valueString = part.trim()
                    if (valueString) {
                        event.body = valueString
                    }
                }
            }

    event
}

def getUUIDFromUSN(usn) {
    def parts = usn.split(":")

    for (int i = 0; i < parts.size(); ++i) {
        if (parts[i] == "uuid") {
            return parts[i + 1]
        }
    }
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

def healthCheck() {
    ssdpDiscover();

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

def apiPost(ip, port, path, query, body, contentType) {
    def url = "${ip}:${port}";
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

private def modifyMac(String macString) {
    return macString.replaceAll(":", "")
}

def debug(message) {
    def debug = false
    if (debug) {
        log.debug message
    }
}
