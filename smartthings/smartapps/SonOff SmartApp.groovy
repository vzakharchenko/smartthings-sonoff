definition(
        name: "SonOff SmartApp",
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
    def devices = searchDeviceWithMac();
    if (!state.clearDevices) {
        state.sonoffMacDevices = [:];
        state.sonoff2MacDevices = [:];
        state.sonoff3MacDevices = [:];
        state.sonoff4MacDevices = [:];
        state.sonoffPowR2MacDevices = [:];
        state.clearDevices = true
    }
    if (!state.subscribe) {
        subscribe(location, "ssdpTerm." + getURN1(), locationHandlerPowR2)
        subscribe(location, "ssdpTerm." + getURNChannels(1), locationHandler)
        subscribe(location, "ssdpTerm." + getURNChannels(2), locationHandler2)
        subscribe(location, "ssdpTerm." + getURNChannels(3), locationHandler3)
        subscribe(location, "ssdpTerm." + getURNChannels(4), locationHandler4)
        state.subscribe = true
    }
    if (state.sonoffMacDevices == null) {
        state.sonoffMacDevices = [:];
    }
    if (state.sonoff2MacDevices == null) {
        state.sonoff2MacDevices = [:];
    }
    if (state.sonoff3MacDevices == null) {
        state.sonoff3MacDevices = [:];
    }
    if (state.sonoff4MacDevices == null) {
        state.sonoff4MacDevices = [:];
    }
    if (state.sonoffPowR2MacDevices == null) {
        state.sonoffPowR2MacDevices = [:];
    }
    int refreshCount = !state.refreshCount ? 0 : state.refreshCount as int
    state.refreshCount = refreshCount + 1
    def refreshInterval = refreshCount == 0 ? 2 : 5
    //ssdp request every fifth refresh
    if ((refreshCount % 2) == 0) {
        ssdpDiscover();
    }

    dynamicPage(name: "config", title: "SonOff Manager", refreshInterval: refreshInterval, install: true, uninstall: true) {
        section("Offline Timeout") {
            input "offlineTimeout", "enum", multiple: false, required: true, title: "Select Offline timeout in minutes", options: ["5", "10", "15", "20", "30", "45", "60", "never"]
        }
        section("on this hub...") {
            input "theHub", "hub", multiple: false, required: true, defaultValue: state.hub
        }

        if (state.sonoffMacDevices == null) {
            state.sonoffMacDevices = [:];
        }
        if (state.sonoff2MacDevices == null) {
            state.sonoff2MacDevices = [:];
        }
        if (state.sonoff3MacDevices == null) {
            state.sonoff3MacDevices = [:];
        }
        if (state.sonoff4MacDevices == null) {
            state.sonoff4MacDevices = [:];
        }
        if (state.sonoffPowR2MacDevices == null) {
            state.sonoffPowR2MacDevices = [:];
        }
        if (state.sonoffDevicesTimes == null) {
            state.sonoffDevicesTimes = [:];
        }
        section("Device List") {
            def deviceOptions = [];

            def consol = [:];
            consol.putAll(state.sonoffMacDevices);
            consol.putAll(state.sonoff2MacDevices);
            consol.putAll(state.sonoff3MacDevices);
            consol.putAll(state.sonoff4MacDevices);
            consol.putAll(state.sonoffPowR2MacDevices);
            debug("consol:$consol")
            debug("devices:$devices")
            consol.each { mac, ip ->


                def device = devices.find { device -> device.getDeviceNetworkId() == mac }
                if (device == null) {
                    deviceOptions.push(mac);
                }
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

public def fromMultiChannel(mac, json) {
    debug("response from ${mac} json = ${json}");
    def subChannels = getAllDevicesByMac(mac);
    debug("subChannels ${subChannels}");
    initChannels(subChannels, json.relay1, json.relay2, json.relay3, json.relay4);
}

def initialize() {
    unsubscribe();
    state.clearDevices = falseж
    state.subscribe = false;
    if (sonoffs) {
        sonoffs.each {
            def mac = it;

            def ip = state.sonoffMacDevices.get(mac);
            if (ip) {
                debug("mac = ${mac}");
                def sonoffDevice = searchDeviceByMac(mac);

                if (sonoffDevice == null) {
                    sonoffDevice = addChildDevice("vzakharchenko", "Sonoff Switch", mac, theHub.id, [label: "Sonoff(${mac})", name: "Sonoff(${mac})"])
                }
                sonoffDevice.setIp(ip);
                sonoffDevice.setPort("80");
                apiPost(ip, 80, "/config", null,
                        "applicationId=${app.id}" +
                                "&accessToken=${state.accessToken}"
                        , "application/x-www-form-urlencoded")
            } else {
                ip = state.sonoffPowR2MacDevices.get(mac);
                if (ip) {
                    debug("mac = ${mac}");
                    def sonoffDevice = searchDeviceByMac(mac);

                    if (sonoffDevice == null) {
                        sonoffDevice = addChildDevice("vzakharchenko", "Sonoff CSE7766 Switch", mac, theHub.id, [label: "Sonoff(${mac}) POWR2", name: "Sonoff(${mac}) POWR2"])
                    }
                    sonoffDevice.setIp(ip);
                    sonoffDevice.setPort("80");
                    apiPost(ip, 80, "/config", null,
                            "applicationId=${app.id}" +
                                    "&accessToken=${state.accessToken}"
                            , "application/x-www-form-urlencoded")
                } else {
                    def sonoffDevice = searchDeviceByMac(mac);
                    ip = state.sonoff2MacDevices.get(mac);
                    if (sonoffDevice == null) {
                        sonoffDevice = addChildDevice("vzakharchenko", "Sonoff MultiChannel", mac, theHub.id, [label: "Sonoff(${mac}) MultiChannel", name: "Sonoff(${mac}) MultiChannel"])
                        sonoffDevice.setChannels("2");
                    }
                    def sonoffDevice1 = searchDeviceByMacAndChannel(mac, 1);
                    if (sonoffDevice1 == null) {
                        sonoffDevice1 = addChildDevice("vzakharchenko", "Sonoff Channel Switch", "CH_1_${mac}", theHub.id, [label: "Sonoff(${mac}) 1", name: "Sonoff(${mac}) 1"])
                        sonoffDevice1.setIp(ip);
                        sonoffDevice1.setPort("80");
                        sonoffDevice1.setMac(mac);
                        sonoffDevice1.setChannel("1");
                    }
                    def sonoffDevice2 = searchDeviceByMacAndChannel(mac, 2);
                    if (sonoffDevice2 == null) {
                        sonoffDevice2 = addChildDevice("vzakharchenko", "Sonoff Channel Switch", "CH_2_${mac}", theHub.id, [label: "Sonoff(${mac}) 2", name: "Sonoff(${mac}) 2"])
                        sonoffDevice2.setIp(ip);
                        sonoffDevice2.setPort("80");
                        sonoffDevice2.setMac(mac);
                        sonoffDevice2.setChannel("2");
                    }
                    def sonoffDevice3 = searchDeviceByMacAndChannel(mac, 3);
                    def sonoffDevice4 = searchDeviceByMacAndChannel(mac, 4);
                    ip = state.sonoff3MacDevices.get(mac);
                    if (ip) {
                        sonoffDevice1.setIp(ip);
                        sonoffDevice2.setIp(ip);
                        sonoffDevice.setChannels("3");
                        if (sonoffDevice3 == null) {
                            sonoffDevice3 = addChildDevice("vzakharchenko", "Sonoff Channel Switch", "CH_3_${mac}", theHub.id, [label: "Sonoff(${mac}) 3", name: "Sonoff(${mac}) 3"])
                            sonoffDevice3.setIp(ip);
                            sonoffDevice3.setPort("80");
                            sonoffDevice3.setMac(mac);
                            sonoffDevice3.setChannel("3");
                        }
                    } else {
                        ip = state.sonoff4MacDevices.get(mac);
                        if (ip) {
                            sonoffDevice1.setIp(ip);
                            sonoffDevice2.setIp(ip);
                            sonoffDevice.setChannels("4");
                            if (sonoffDevice3 == null) {
                                sonoffDevice3 = addChildDevice("vzakharchenko", "Sonoff Channel Switch", "CH_3_${mac}", theHub.id, [label: "Sonoff(${mac}) 3", name: "Sonoff(${mac}) 3"])
                                sonoffDevice3.setIp(ip);
                                sonoffDevice3.setPort("80");
                                sonoffDevice3.setMac(mac);
                                sonoffDevice3.setChannel("3");
                            }
                            if (sonoffDevice4 == null) {
                                sonoffDevice4 = addChildDevice("vzakharchenko", "Sonoff Channel Switch", "CH_4_${mac}", theHub.id, [label: "Sonoff(${mac}) 4", name: "Sonoff(${mac}) 4"])
                                sonoffDevice4.setIp(ip);
                                sonoffDevice4.setPort("80");
                                sonoffDevice4.setMac(mac);
                                sonoffDevice4.setChannel("4");
                            }
                        }
                    }
                    sonoffDevice.setIp(ip);
                    sonoffDevice.setPort("80");
                    apiPost(ip, 80, "/config", null,
                            "applicationId=${app.id}" +
                                    "&accessToken=${state.accessToken}"
                            , "application/x-www-form-urlencoded")
                }
            }


        }
    }
    if (!state.subscribe) {
        subscribe(location, "ssdpTerm." + getURN1(), locationHandlerPowR2)
        subscribe(location, "ssdpTerm." + getURNChannels(1), locationHandler)
        subscribe(location, "ssdpTerm." + getURNChannels(2), locationHandler2)
        subscribe(location, "ssdpTerm." + getURNChannels(3), locationHandler3)
        subscribe(location, "ssdpTerm." + getURNChannels(4), locationHandler4)
        state.subscribe = true

    }
    if (offlineTimeout == "never") {
        state.offlineTimeOut = 0;
    } else {
        state.offlineTimeOut = offlineTimeout.toInteger();
    }
    runEvery5Minutes(healthCheck)
}


mappings {
    path("/init") {
        action:
        [
                GET: "init"
        ]
    }
    path("/get/subscribe") {
        action:
        [
                GET: "subscribe"
        ]
    }

    path("/get/info") {
        action:
        [
                GET: "infoGET"
        ]
    }
}

def initChannels(devicesByMac, relay1, relay2, relay3, relay4) {
    def sonoffDeviceChannel1 = devicesByMac.get("1");
    if (sonoffDeviceChannel1 != null) {
        debug("sonoffDeviceChannel1 ${sonoffDeviceChannel1} ${relay1}");
        if (relay1.equals("on")) {
            sonoffDeviceChannel1.forceOn();
        } else if (relay1.equals("off")) {
            sonoffDeviceChannel1.forceOff();
        }
    }
    def sonoffDeviceChannel2 = devicesByMac.get("2");
    if (sonoffDeviceChannel2 != null) {
        debug("sonoffDeviceChannel2 ${sonoffDeviceChannel2} ${relay2}");
        if (relay2.equals("on")) {
            sonoffDeviceChannel2.forceOn();
        } else if (relay2.equals("off")) {
            sonoffDeviceChannel2.forceOff();
        }
    }
    def sonoffDeviceChannel3 = devicesByMac.get("3");
    if (sonoffDeviceChannel3 != null) {
        debug("sonoffDeviceChannel3 ${sonoffDeviceChannel3} ${relay3}");
        if (relay3.equals("on")) {
            sonoffDeviceChannel3.forceOn();
        } else if (relay3.equals("off")) {
            sonoffDeviceChannel3.forceOff();
        }
    }
    def sonoffDeviceChannel4 = devicesByMac.get("4");
    if (sonoffDeviceChannel4 != null) {
        debug("sonoffDeviceChannel4 ${sonoffDeviceChannel4} ${relay4}");
        if (relay4.equals("on")) {
            sonoffDeviceChannel4.forceOn();
        } else if (relay4.equals("off")) {
            sonoffDeviceChannel4.forceOff();
        }
    }
}

def initAdresssChannels(devicesByMac, mac, ip, port) {
    def sonoffDeviceChannel1 = devicesByMac.get("1");
    if (sonoffDeviceChannel1 != null) {
        sonoffDeviceChannel1.setIp(ip);
        sonoffDeviceChannel1.setPort(port);
    }
    def sonoffDeviceChannel2 = devicesByMac.get("2");
    if (sonoffDeviceChannel2 != null) {
        sonoffDeviceChannel2.setIp(ip);
        sonoffDeviceChannel2.setPort(port);
    }
    def sonoffDeviceChannel3 = devicesByMac.get("3");
    if (sonoffDeviceChannel3 != null) {
        sonoffDeviceChannel3.setIp(ip);
        sonoffDeviceChannel3.setPort(port);
    }
    def sonoffDeviceChannel4 = devicesByMac.get("4");
    if (sonoffDeviceChannel4 != null) {
        sonoffDeviceChannel4.setIp(ip);
        sonoffDeviceChannel4.setPort(port);
    }
}


def init() {
    def mac = modifyMac(params.mac);

    def sonoffDevice = searchDeviceByMac(mac);
    if (sonoffDevice != null) {
        if (sonoffDevice.getTypeName() == "Sonoff MultiChannel") {
            def relay1 = params.relay1;
            def relay2 = params.relay2;
            def relay3 = params.relay3;
            def relay4 = params.relay4;
            def devicesByMac = getAllDevicesByMac(mac);
            initChannels(devicesByMac, relay1, relay2, relay3, relay4)
        } else {
            def relay1 = params.relay1;
            debug("init One Channel Sonoff: ${params.ip}:${sonoffDevice.getDeviceNetworkId()}:${relay1}");
            if (relay1.equals("on")) {
                sonoffDevice.forceOn();
            } else if (relay1.equals("off")) {
                sonoffDevice.forceOff();
            }
        }
        sonoffDevice.markDeviceOnline();
        updateActiveTime(mac)
        sonoffDevice.setIp(params.ip);
        // sonoffDevice.subscribeCommand();
    }
    debug("init: $params")
    return "OK"
}

def updateActiveTime(mac) {
    def date = new Date();
    debug("update time for ${mac} states: ${state.sonoffDevicesTimes}");
    def sonoffDevice = searchDeviceByMac(mac);
    if (sonoffDevice != null) {
        sonoffDevice.markDeviceOnline();
    }
    state.sonoffDevicesTimes.put(mac, date.getTime());
}


def subscribe() {
    def json = request.JSON;
    def mac = modifyMac(params.mac);
    def sonoffDevice = searchDeviceByMac(mac);
    if (sonoffDevice) {
        sonoffDevice.markDeviceOnline();
        updateActiveTime(mac);
        sonoffDevice.setIp(params.ip);
        sonoffDevice.subscribeCommand();
    }
    return "OK"
}

def infoGET() {
    def s = "undefined";
    def name = "undefined";
    def deviceName = "undefined";
    debug("params=${params}");
    def mac = modifyMac(params.mac);
    def ch = params.ch;
    if (ch == null) {
        ch = 1;
    }
    debug("mac=${mac}");
    def sonoffDevice = searchDeviceByMacAndChannel(mac, ch);
    if (sonoffDevice != null) {
        updateActiveTime(mac);
        def switchData = sonoffDevice.currentState("switch");
        if (switchData) {
            s = switchData.value
            name = switchData.linkText;
        }
        deviceName = sonoffDevice.getLabel();
        sonoffDevice.setIp(params.ip);
    }
    return [status: s, name: name, deviceName: deviceName]
}

def getURN1() {
    return "urn:sonoff:device:e:1:vassio"
}

def getURNChannels(channels) {
    return "urn:sonoff:device:" + channels + ":vassio";
}

void ssdpDiscover() {
    debug("send lan discovery " + getURN1())
    sendHubCommand(new physicalgraph.device.HubAction("lan discovery " + getURN1(), physicalgraph.device.Protocol.LAN))
    debug("send lan discovery " + getURNChannels(1))
    sendHubCommand(new physicalgraph.device.HubAction("lan discovery " + getURNChannels(1), physicalgraph.device.Protocol.LAN))
    debug("send lan discovery " + getURNChannels(2))
    sendHubCommand(new physicalgraph.device.HubAction("lan discovery " + getURNChannels(2), physicalgraph.device.Protocol.LAN))
    debug("send lan discovery " + getURNChannels(3))
    sendHubCommand(new physicalgraph.device.HubAction("lan discovery " + getURNChannels(3), physicalgraph.device.Protocol.LAN))
    debug("send lan discovery " + getURNChannels(4))
    sendHubCommand(new physicalgraph.device.HubAction("lan discovery " + getURNChannels(4), physicalgraph.device.Protocol.LAN))
}

def locationHandler(evt) {
    def description = evt?.description
    debug("event: ${description}");
    def urn = getURNChannels(1)
    def hub = evt?.hubId
    def parsedEvent = parseEventMessage(description)
    state.hub = hub
    if (parsedEvent?.ssdpTerm?.contains(urn)) {
        if (state.sonoffMacDevices == null) {
            state.sonoffMacDevices = [:];
        }
        def ip = convertHexToIP(parsedEvent.ip)
        state.sonoffMacDevices.put(modifyMac(parsedEvent.mac), ip);
        checkSonOff(parsedEvent, modifyMac(parsedEvent.mac));
    }
}

def locationHandler2(evt) {
    def description = evt?.description
    debug("event: ${description}");
    def urn = getURNChannels(2)
    def hub = evt?.hubId
    def parsedEvent = parseEventMessage(description)
    state.hub = hub
    if (parsedEvent?.ssdpTerm?.contains(urn)) {
        if (state.sonoff2MacDevices == null) {
            state.sonoff2MacDevices = [:];
        }
        def ip = convertHexToIP(parsedEvent.ip)
        state.sonoff2MacDevices.put(modifyMac(parsedEvent.mac), ip);
        checkSonOff(parsedEvent, modifyMac(parsedEvent.mac));
    }
}

def locationHandler3(evt) {
    def description = evt?.description
    debug("event: ${description}");
    def urn = getURNChannels(3)
    def hub = evt?.hubId
    def parsedEvent = parseEventMessage(description)
    state.hub = hub
    if (parsedEvent?.ssdpTerm?.contains(urn)) {
        if (state.sonoff3MacDevices == null) {
            state.sonoff3MacDevices = [:];
        }
        def ip = convertHexToIP(parsedEvent.ip)
        state.sonoff3MacDevices.put(modifyMac(parsedEvent.mac), ip);
        checkSonOff(parsedEvent, modifyMac(parsedEvent.mac));
    }
}

def locationHandler4(evt) {
    def description = evt?.description
    debug("event: ${description}");
    def urn = getURNChannels(4)
    def hub = evt?.hubId
    def parsedEvent = parseEventMessage(description)
    state.hub = hub
    if (parsedEvent?.ssdpTerm?.contains(urn)) {
        if (state.sonoff4MacDevices == null) {
            state.sonoff4MacDevices = [:];
        }
        def ip = convertHexToIP(parsedEvent.ip)
        state.sonoff4MacDevices.put(modifyMac(parsedEvent.mac), ip);
        checkSonOff(parsedEvent, modifyMac(parsedEvent.mac));
    }
}

def locationHandlerPowR2(evt) {
    def description = evt?.description
    debug("event: ${description}");
    def urn = getURN1()
    def hub = evt?.hubId
    def parsedEvent = parseEventMessage(description)
    state.hub = hub
    if (parsedEvent?.ssdpTerm?.contains(urn)) {
        if (state.sonoffPowR2MacDevices == null) {
            state.sonoffPowR2MacDevices = [:];
        }
        def ip = convertHexToIP(parsedEvent.ip)
        state.sonoffPowR2MacDevices.put(modifyMac(parsedEvent.mac), ip);
        checkSonOff(parsedEvent, modifyMac(parsedEvent.mac));
    }
}

def checkSonOff(parsedEvent, mac) {
    def curTime = new Date().getTime();
    def device = searchDeviceByMac(mac);

    if (device) {
        if (device.getTypeName() == "Sonoff MultiChannel") {
            def devicesByMac = getAllDevicesByMac(mac);
            initAdresssChannels(devicesByMac, mac, convertHexToIP(parsedEvent.ip), convertHexToInt(parsedEvent.port));

        }
        device.setIp(convertHexToIP(parsedEvent.ip));
        device.setPort(convertHexToInt(parsedEvent.port));
        state.sonoffDevicesTimes.put(modifyMac(parsedEvent.mac), curTime)
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
    getAllDevices().each {
        if (it.getTypeName() == devType) {
            typeDevices.add(it)
        }
    }
    return typeDevices
}

def searchDeviceWithMac() {
    def device = getAllDevices().find {
        if (!it.getDeviceNetworkId().contains("CH_")) {
            return it;
        }
    }
    return device;
}

def searchDeviceByMac(mac) {
    def device = getAllDevices().find {
        if (it.getDeviceNetworkId() == mac) {
            return it;
        }
    }
    return device;
}

def searchDeviceByMacAndChannel(mac, channel) {
    def device = getAllDevices().find {
        if (it.getDeviceNetworkId() == "CH_${channel}_${mac}") {
            return it;
        }
    }
    return device;
}

def getAllDevicesByMac(mac) {
    def deviceMap = [:];
    def devices = getAllDevices().each {
        if (it.getDeviceNetworkId().contains(mac)) {
            if (it.getDeviceNetworkId() != mac) {
                def devChannels = it.getDeviceNetworkId().split("_");
                deviceMap.put(devChannels[1], it);
            }

        }
    }
    return deviceMap;
}

def getAllDevices() {
    return childDevices;
}

def healthCheck() {
    ssdpDiscover();
    def timeout = 1000 * 60 * state.offlineTimeOut;
    def curTime = new Date().getTime();


    def devices = searchDeviceWithMac();


    devices.each {
        if (timeout > 0) {
            def mac = it.getDeviceNetworkId();
            def ip = state.sonoffMacDevices.get(mac);
            def activeDate = state.sonoffDevicesTimes.get(mac);
            if ((curTime - timeout) > activeDate) {
                it.markDeviceOffline();
                debug("ip ${ip} offline ${curTime - timeout} > ${activeDate} ")
            } else {
                it.markDeviceOnline();
                debug("ip ${ip} online ${curTime - timeout} < ${activeDate} ")
            }
        } else {
            it.markDeviceOnline();
        }
    }
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
