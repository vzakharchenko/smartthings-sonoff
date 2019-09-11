
#ifndef SSDP_cpp
#define SSDP_cpp
/*
  ESP8266 Simple Service Discovery
  Copyright (c) 2015 Hristo Gochkov

  Original (Arduino) version by Filippo Sallemi, July 23, 2014.
  Can be found at: https://github.com/nomadnt/uSSDP

  License (MIT license):
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*/
#ifndef LWIP_OPEN_SRC
#define LWIP_OPEN_SRC
#endif
#include <functional>
#include "SSDP.h"
#include "WiFiUdp.h"
#include "debug.h"

extern "C" {
#include "osapi.h"
#include "ets_sys.h"
#include "user_interface.h"
}

#include "lwip/opt.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/mem.h"
#include "include/UdpContext.h"

// #define DEBUG_SSDP  Serial

#define SSDP_INTERVAL     1200
#define SSDP_PORT         1900
#define SSDP_METHOD_SIZE  10
#define SSDP_URI_SIZE     2
#define SSDP_BUFFER_SIZE  64
#define SSDP_MULTICAST_TTL 2
static const IPAddress SSDP_MULTICAST_ADDR(239, 255, 255, 250);



static const char _ssdp_response_template[] PROGMEM =
  "HTTP/1.1 200 OK\r\n"
  "EXT:\r\n";

static const char _ssdp_notify_template[] PROGMEM =
  "NOTIFY * HTTP/1.1\r\n"
  "HOST: 239.255.255.250:1900\r\n"
  "NTS: ssdp:alive\r\n";

static const char _ssdp_packet_template[] PROGMEM =
  "%s" // _ssdp_response_template / _ssdp_notify_template
  "CACHE-CONTROL: max-age=%u\r\n" // SSDP_INTERVAL
  "SERVER: Arduino/1.0 UPNP/1.1 %s/%s\r\n" // _modelName, _modelNumber
  "USN: uuid:%s\r\n" // _uuid
  "%s: %s\r\n"  // "NT" or "ST", _deviceType
  "LOCATION: http://%u.%u.%u.%u:%u/%s\r\n" // WiFi.localIP(), _port, _schemaURL
  "\r\n";

static const char _ssdp_event_packet_template[] PROGMEM =
  "NOTIFY %s HTTP/1.1\r\n" // CALLBACK
  "HOST: %s\r\n" //HOST
  "NT: upnp:event\r\n"
  "NTS:  upnp:propchange\r\n"
  "SID:  uuid:%s\r\n"
  "CONTENT-LENGTH: %u\r\n" // LENGTH
  "SEQ: %u\r\n"//UNIQ_ID
  "SERVER: Arduino/1.0 UPNP/1.1 %s/%s\r\n" // _modelName, _modelNumber
  "\r\n"
  "%s";

static const char _ssdp_schema_template[] PROGMEM =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/xml\r\n"
  "Connection: close\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "\r\n"
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
  "<specVersion>"
  "<major>1</major>"
  "<minor>0</minor>"
  "</specVersion>"
  "<URLBase>http://%u.%u.%u.%u:%u/</URLBase>" // WiFi.localIP(), _port
  "<device>"
  "<deviceType>%s</deviceType>"
  "<friendlyName>%s</friendlyName>"
  "<presentationURL>%s</presentationURL>"
  "<serialNumber>%s</serialNumber>"
  "<modelName>%s</modelName>"
  "<modelNumber>%s</modelNumber>"
  "<modelURL>%s</modelURL>"
  "<manufacturer>%s</manufacturer>"
  "<manufacturerURL>%s</manufacturerURL>"
  "<UDN>uuid:%s</UDN>"
  "</device>"
  //    "<iconList>"
  //      "<icon>"
  //        "<mimetype>image/png</mimetype>"
  //        "<height>48</height>"
  //        "<width>48</width>"
  //        "<depth>24</depth>"
  //        "<url>icon48.png</url>"
  //      "</icon>"
  //      "<icon>"
  //       "<mimetype>image/png</mimetype>"
  //       "<height>120</height>"
  //       "<width>120</width>"
  //       "<depth>24</depth>"
  //       "<url>icon120.png</url>"
  //      "</icon>"
  //    "</iconList>"
  "</root>\r\n"
  "\r\n";


struct SSDPTimer {
  ETSTimer timer;
};

SSDPClass2::SSDPClass2() :
  _server(0),
  _timer(new SSDPTimer),
  _port(80),
  _ttl(SSDP_MULTICAST_TTL),
  _respondToPort(0),
  _pending(false),
  _delay(0),
  _process_time(0),
  _notify_time(0)
{
  _uuid[0] = '\0';
  _modelNumber[0] = '\0';
  sprintf(_deviceType, "urn:schemas-upnp-org:device:Basic:1");
  _friendlyName[0] = '\0';
  _presentationURL[0] = '\0';
  _serialNumber[0] = '\0';
  _modelName[0] = '\0';
  _modelURL[0] = '\0';
  _manufacturer[0] = '\0';
  _manufacturerURL[0] = '\0';
  sprintf(_schemaURL, "ssdp/schema.xml");
}

SSDPClass2::~SSDPClass2() {
  delete _timer;
}

bool SSDPClass2::begin() {
  _pending = false;

  uint32_t chipId = ESP.getChipId();
  sprintf(_uuid, "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
          (uint16_t) ((chipId >> 16) & 0xff),
          (uint16_t) ((chipId >>  8) & 0xff),
          (uint16_t)   chipId        & 0xff  );

#ifdef DEBUG_SSDP
  DEBUG_SSDP.printf("SSDP UUID: %s\n", (char *)_uuid);
#endif

  if (_server) {
    _server->unref();
    _server = 0;
  }

  _server = new UdpContext;
  _server->ref();

  ip_addr_t ifaddr;
  ifaddr.addr = WiFi.localIP();
  ip_addr_t multicast_addr;
  multicast_addr.addr = (uint32_t) SSDP_MULTICAST_ADDR;
  if (igmp_joingroup(&ifaddr, &multicast_addr) != ERR_OK ) {
    DEBUGV("SSDP failed to join igmp group");
    return false;
  }

  if (!_server->listen(*IP_ADDR_ANY, SSDP_PORT)) {
    return false;
  }

  _server->setMulticastInterface(ifaddr);
  _server->setMulticastTTL(_ttl);
  _server->onRx(std::bind(&SSDPClass2::_update, this));
  if (!_server->connect(multicast_addr, SSDP_PORT)) {
    return false;
  }

  _startTimer();

  return true;
}

String SSDPClass2::getUuid() {
  return String(_uuid);
}

void SSDPClass2::sendEvent(String xml, String host, int port, String action, int seq) {
  char _xml[2048];
  char _path[1024];
  char _url[1024];
  char _host[512];
  String url = String(host) + ":" + String(port);
  url.toCharArray(_url, url.length() + 1);
  action.toCharArray(_path, action.length() + 1);
  xml.toCharArray(_xml, xml.length() + 1);
  host.toCharArray(_host, host.length() + 1);
  //  strlcpy(_xml, xml, sizeof(xml));
  //  strlcpy(_path, path, sizeof(path));
  //  strlcpy(_url, url, sizeof(url));
  char buffer[2048];
  IPAddress ip = WiFi.localIP();

  char valueBuffer[strlen_P(_ssdp_event_packet_template) + 1];
  strcpy_P(valueBuffer, _ssdp_event_packet_template);

  int len = snprintf_P(buffer, sizeof(buffer),
                       valueBuffer,
                       _path,
                       _url,
                       sizeof(xml),
                       seq,
                       _uuid,
                       _modelName, _modelNumber,
                       _xml
                      );

  _server->append(buffer, len);
  ip_addr_t remoteAddr;
  uint16_t remotePort;
  remoteAddr.addr = inet_addr(_host);
  remotePort = port;

  _server->send(&remoteAddr, remotePort);
}

void SSDPClass2::_send(ssdp_method_t method) {
  char buffer[1460];
  IPAddress ip = WiFi.localIP();

  char valueBuffer[strlen_P(_ssdp_notify_template) + 1];
  strcpy_P(valueBuffer, (method == NONE) ? _ssdp_response_template : _ssdp_notify_template);

  int len = snprintf_P(buffer, sizeof(buffer),
                       _ssdp_packet_template,
                       valueBuffer,
                       SSDP_INTERVAL,
                       _modelName, _modelNumber,
                       _uuid,
                       (method == NONE) ? "ST" : "NT",
                       _deviceType,
                       ip[0], ip[1], ip[2], ip[3], _port, _schemaURL
                      );

  _server->append(buffer, len);

  ip_addr_t remoteAddr;
  uint16_t remotePort;
  if (method == NONE) {
    remoteAddr.addr = _respondToAddr;
    remotePort = _respondToPort;
#ifdef DEBUG_SSDP
    DEBUG_SSDP.print("Sending Response to ");
#endif
  } else {
    remoteAddr.addr = SSDP_MULTICAST_ADDR;
    remotePort = SSDP_PORT;
#ifdef DEBUG_SSDP
    DEBUG_SSDP.println("Sending Notify to ");
#endif
  }
#ifdef DEBUG_SSDP
  DEBUG_SSDP.print(IPAddress(remoteAddr.addr));
  DEBUG_SSDP.print(":");
  DEBUG_SSDP.println(remotePort);
#endif

  _server->send(&remoteAddr, remotePort);
}

void SSDPClass2::schema(WiFiClient client) {
  IPAddress ip = WiFi.localIP();
  char buffer[strlen_P(_ssdp_schema_template) + 1];
  strcpy_P(buffer, _ssdp_schema_template);
  client.printf(buffer,
                ip[0], ip[1], ip[2], ip[3], _port,
                _deviceType,
                _friendlyName,
                _presentationURL,
                _serialNumber,
                _modelName,
                _modelNumber,
                _modelURL,
                _manufacturer,
                _manufacturerURL,
                _uuid
               );
}

void SSDPClass2::_update() {
  if (!_pending && _server->next()) {
    ssdp_method_t method = NONE;

    _respondToAddr = _server->getRemoteAddress();
    _respondToPort = _server->getRemotePort();

    typedef enum {METHOD, URI, PROTO, KEY, VALUE, ABORT} states;
    states state = METHOD;

    typedef enum {START, MAN, ST, MX} headers;
    headers header = START;

    uint8_t cursor = 0;
    uint8_t cr = 0;

    char buffer[SSDP_BUFFER_SIZE] = {0};

    while (_server->getSize() > 0) {
      char c = _server->read();

      (c == '\r' || c == '\n') ? cr++ : cr = 0;

      switch (state) {
        case METHOD:
          if (c == ' ') {
            if (strcmp(buffer, "M-SEARCH") == 0) method = SEARCH;

            if (method == NONE) state = ABORT;
            else state = URI;
            cursor = 0;

          } else if (cursor < SSDP_METHOD_SIZE - 1) {
            buffer[cursor++] = c;
            buffer[cursor] = '\0';
          }
          break;
        case URI:
          if (c == ' ') {
            if (strcmp(buffer, "*")) state = ABORT;
            else state = PROTO;
            cursor = 0;
          } else if (cursor < SSDP_URI_SIZE - 1) {
            buffer[cursor++] = c;
            buffer[cursor] = '\0';
          }
          break;
        case PROTO:
          if (cr == 2) {
            state = KEY;
            cursor = 0;
          }
          break;
        case KEY:
          if (cr == 4) {
            _pending = true;
            _process_time = millis();
          }
          else if (c == ' ') {
            cursor = 0;
            state = VALUE;
          }
          else if (c != '\r' && c != '\n' && c != ':' && cursor < SSDP_BUFFER_SIZE - 1) {
            buffer[cursor++] = c;
            buffer[cursor] = '\0';
          }
          break;
        case VALUE:
          if (cr == 2) {
            switch (header) {
              case START:
                break;
              case MAN:
#ifdef DEBUG_SSDP
                DEBUG_SSDP.printf("MAN: %s\n", (char *)buffer);
#endif
                break;
              case ST:
                if (strcmp(buffer, "ssdp:all")) {
                  state = ABORT;
#ifdef DEBUG_SSDP
                  DEBUG_SSDP.printf("REJECT: %s\n", (char *)buffer);
#endif
                }
                // if the search type matches our type, we should respond instead of ABORT
                if (strcasecmp(buffer, _deviceType) == 0) {
                  _pending = true;
                  _process_time = millis();
                  state = KEY;
                }
                break;
              case MX:
                _delay = random(0, atoi(buffer)) * 1000L;
                break;
            }

            if (state != ABORT) {
              state = KEY;
              header = START;
              cursor = 0;
            }
          } else if (c != '\r' && c != '\n') {
            if (header == START) {
              if (strncmp(buffer, "MA", 2) == 0) header = MAN;
              else if (strcmp(buffer, "ST") == 0) header = ST;
              else if (strcmp(buffer, "MX") == 0) header = MX;
            }

            if (cursor < SSDP_BUFFER_SIZE - 1) {
              buffer[cursor++] = c;
              buffer[cursor] = '\0';
            }
          }
          break;
        case ABORT:
          _pending = false; _delay = 0;
          break;
      }
    }
  }

  if (_pending && (millis() - _process_time) > _delay) {
    _pending = false; _delay = 0;
    _send(NONE);
  } else if (_notify_time == 0 || (millis() - _notify_time) > (SSDP_INTERVAL * 1000L)) {
    _notify_time = millis();
    _send(NOTIFY);
  }

  if (_pending) {
    while (_server->next())
      _server->flush();
  }

}

void SSDPClass2::setSchemaURL(const char *url) {
  strlcpy(_schemaURL, url, sizeof(_schemaURL));
}

void SSDPClass2::setHTTPPort(uint16_t port) {
  _port = port;
}

void SSDPClass2::setDeviceType(const char *deviceType) {
  strlcpy(_deviceType, deviceType, sizeof(_deviceType));
}

void SSDPClass2::setName(const char *name) {
  strlcpy(_friendlyName, name, sizeof(_friendlyName));
}

void SSDPClass2::setURL(const char *url) {
  strlcpy(_presentationURL, url, sizeof(_presentationURL));
}

void SSDPClass2::setSerialNumber(const char *serialNumber) {
  strlcpy(_serialNumber, serialNumber, sizeof(_serialNumber));
}

void SSDPClass2::setSerialNumber(const uint32_t serialNumber) {
  snprintf(_serialNumber, sizeof(uint32_t) * 2 + 1, "%08X", serialNumber);
}

void SSDPClass2::setModelName(const char *name) {
  strlcpy(_modelName, name, sizeof(_modelName));
}

void SSDPClass2::setModelNumber(const char *num) {
  strlcpy(_modelNumber, num, sizeof(_modelNumber));
}

void SSDPClass2::setModelURL(const char *url) {
  strlcpy(_modelURL, url, sizeof(_modelURL));
}

void SSDPClass2::setManufacturer(const char *name) {
  strlcpy(_manufacturer, name, sizeof(_manufacturer));
}

void SSDPClass2::setManufacturerURL(const char *url) {
  strlcpy(_manufacturerURL, url, sizeof(_manufacturerURL));
}

void SSDPClass2::setTTL(const uint8_t ttl) {
  _ttl = ttl;
}

void SSDPClass2::_onTimerStatic(SSDPClass2* self) {
  self->_update();
}

void SSDPClass2::_startTimer() {
  ETSTimer* tm = &(_timer->timer);
  const int interval = 1000;
  os_timer_disarm(tm);
  os_timer_setfn(tm, reinterpret_cast<ETSTimerFunc*>(&SSDPClass2::_onTimerStatic), reinterpret_cast<void*>(this));
  os_timer_arm(tm, interval, 1 /* repeat */);
}

#endif
