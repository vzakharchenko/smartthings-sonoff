/**
 *  Copyright 2015 SmartThings
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 *
 */
metadata {
    definition(name: "Door Shield", namespace: "smartthings", author: "SmartThings") {
        capability "Actuator"
        capability "Sensor"
        capability "Switch"
        capability "Door Control"
        capability "Lock Only"

        attribute "intercom", "String"

        command "incommingCall"
        command "endCall"
    }

    // simulator metadata
    simulator {
    }

    // UI tile definitions
    tiles {
        standardTile("shield", "device.lock", width: 3, height: 2, canChangeBackground: true) {
            state(name: "locked", label: "open", action: "lock", icon: "st.locks.lock.unlocked", backgroundColor: "#00A0DC")
        }

        main "shield"
        details "shield"
    }
}

// handle commands
def open() {
    lock()
}

def close() {

}

def lock() {
    sendEvent(name: "door", value: "open", isStateChange: true)
    sendEvent(name: "lock", value: "unlocked", isStateChange: true)
    sendEvent(name: "switch", value: "on", isStateChange: true)
}


def on() {
    lock()
}

def off() {
}

def incommingCall() {
    sendEvent(name: "intercom", value: "calling")
}

def endCall() {
    sendEvent(name: "intercom", value: "end")
}
