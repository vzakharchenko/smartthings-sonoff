#ifndef keycloak_h
#define keycloak_h

#include <base64.h>

String SESSION_NAME = "ESP_SESSION";

typedef struct
{
  char sessionId[128];
  char refreshToken[1024];
  char accessToken[1024];
} SessionStorage;

class Keycloak
{
  private:

    int port;
    ESP8266WebServer* server;

    String realm = "SmartHome";
    String authServerUrl = "https://login.zaskarius.in.ua/auth/";
    String resource = "sonoff-device";
    String secret = "04e39914-a6f6-4519-a108-45a66f213c9c";

    SessionStorage sessionStorage{
      "",
      "",
      ""
    };

    String getCookieValue(String cookie, String cookieName) {
      String str = cookieName + "=";
      int i = cookie.indexOf(str);
      if (i != -1) {
        String substring = cookie.substring(i + str.length());
        int j = substring.indexOf(";");
        if (j != -1) {
          substring = substring.substring(0, j);
        }
        return substring;
      }
      return "";
    }

    boolean isSessionValid(String session) {

      return  String(session) != "" && String(session) == String(this->sessionStorage.sessionId);
    }

    String currentHost() {
      String ip = IpAddress2String( WiFi.localIP());
      int port = this->port;
      String http = "http";
      if (port == 443 || port == 8443) {
        http = "https";
      }
      if (port == 443 || port == 80) {
        return http + String("://") + ip + String("/");
      } else {
        return http + String("://") + ip + String(":") + String(port) + String("/");
      }
    }

    boolean refreshActiveToken() {
      HTTPClient http;
      String url = String(this->authServerUrl) + String("realms/") + String(this->realm) + "/protocol/openid-connect/token";
      Serial.println ( "Starting refresh Token Http request : " + url );
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
      client->setInsecure();
      http.begin(*client, url);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      yield();
      int httpCode = http.POST(String("refresh_token=")
                               + String(this->sessionStorage.refreshToken)
                               + String("&grant_type=refresh_token&client_id=")
                               + String(this->resource)
                               + String("&client_secret=")
                               + String(this->secret)
                              );
      yield();
      Serial.println ( "Starting  refresh Token  Http code : " + String(httpCode) );
      Serial.println ( " refresh Token  eand request" );
      String payload = http.getString();
      if (httpCode > 199 && httpCode < 300) {
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        String access_token = doc["access_token"];
        String refresh_token = doc["refresh_token"];
        if (refresh_token != "") {
          saveTokens(access_token, refresh_token);
        }
        http.end();
        return true;

      } else {
        Serial.println ( "Error exchange Token  end request payload" + payload );
        http.end();
        return false;
      }
    }

    boolean isTokenValid(String session) {
      String token = String(this->sessionStorage.accessToken);
      Serial.println ( "token : " + token );
      if (isSessionValid(session) &&  token != "") {

        HTTPClient http;
        String url = String(this->authServerUrl) + String("realms/") + String(this->realm) + "/protocol/openid-connect/token/introspect";
        Serial.println ( "Starting validate Token Http request : " + url );
        String authorization = String(base64::encode(this->resource + String(":") + this->secret));
        Serial.println ( "authorization : " + authorization );
        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        client->setInsecure();
        http.begin(*client, url);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        http.addHeader("Authorization",  authorization);

        String postData = String("token=")
                          + String(token)
                          + String("&client_id=")
                          + String(this->resource)
                          + String("&client_secret=")
                          + String(this->secret);
        yield();
        Serial.println ( "postData : " + postData );
        int code = http.POST(postData);
        yield();
        Serial.println ( "Starting  validate Token  Http code : " + String(code) );
        Serial.println ( " validate Token  end request" );
        String payload = http.getString();
        Serial.println ( " validate Token  end request payload" + payload );
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        String active = doc["active"];
        http.end();
        return active == "true";
      } else {
        return false;
      }
    }

    boolean exchangeCodeToToken(String code) {
      if (String(code) != "") {
        HTTPClient http;
        String url = String(this->authServerUrl) + String("realms/") + String(this->realm) + "/protocol/openid-connect/token";
        Serial.println ( "Starting exchange Token Http request : " + url );
        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
        client->setInsecure();
        client->setBufferSizes(8000, 4000);
        http.begin(*client, url);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        yield();
        String postData = String("code=")
                          + String(code)
                          + String("&grant_type=authorization_code&client_id=")
                          + String(this->resource)
                          + String("&client_secret=")
                          + String(this->secret)
                          + String("&redirect_uri=")
                          + currentHost()
                          + String("callback");
        Serial.println ( " exchange Token  post data: " + String(postData));

        int httpCode = http.POST(postData);

        Serial.println ( "Starting  exchange Token  Http code : " + String(httpCode) );
        String payload = http.getString();
        Serial.println ( "payload" + String(payload) );
        if (httpCode > 199 && httpCode < 300) {
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, payload);

          String access_token = doc["access_token"];
          String refresh_token = doc["refresh_token"];
          Serial.println ( " refresh_token: " + String(refresh_token));
          //          JsonObject root = doc.as<JsonObject>();
          //          String access_token = root["access_token"];
          //          String refresh_token = root["refresh_token"];
          if (refresh_token != "" && access_token != "") {
            this->saveTokens(access_token, refresh_token);
          } else {
            Serial.println ( "access and refresh Token are Empty" );
          }
          http.end();
          return true;
        } else {
          Serial.println ( "Error exchange Token  end request payload" + String(payload) );
          http.end();
          return false;
        }
      } else {
        return false;
      }
    }

    void saveTokens(String access_token, String refresh_token) {
      String sessionId = String(millis());
      sessionId.toCharArray(this->sessionStorage.sessionId, sessionId.length() + 1);
      this->server->sendHeader("Set-Cookie", SESSION_NAME + String("=") + String(this->sessionStorage.sessionId) + String("; Path=/;")
                              );
      refresh_token.toCharArray(this->sessionStorage.refreshToken, refresh_token.length() + 1);
      Serial.println ( "Store Refresh token");
      access_token.toCharArray(this->sessionStorage.accessToken, access_token.length() + 1);
      Serial.println ( "Store Access token: " + access_token + ":" + String(this->sessionStorage.accessToken) );
    }

    void callBackHandle() {
      Serial.println ( " excahnge Token" );
      String code = this->server->arg("code");
      Serial.println ( " Code " + code );
      if (String(code) == "") {
        this->server->send(500, "text/plain", "Code does not exist");
      } else {
        exchangeCodeToToken(code);
        this->redirectToRootPage();
      }

    }

    void redirectToLoginPage() {
      this->server->sendHeader("Location",
                               String(this->authServerUrl)
                               + String("/realms/")
                               + String(this->realm)
                               + String("/protocol/openid-connect/auth?client_id=")
                               + String(this->resource) + String("&redirect_uri=")
                               + currentHost()
                               + String("callback&response_type=code&scope=openid")
                               , true);
      this->server->send(302, "text/plain", "Login Page");
    }

    void redirectToRootPage() {
      this->server->sendHeader("Location",
                               currentHost()
                               , true);
      this->server->send(302, "text/plain", "Login Page");
    }
  public:

    Keycloak(int port) {
      this->port = port;
    }

    boolean isAuthorized() {
      Serial.println("Enter is_authentified");
      if (this->server->hasHeader("Cookie")) {
        Serial.print("Found cookie: ");
        String cookie = this->server->header("Cookie");
        String session = getCookieValue(cookie, SESSION_NAME);
        Serial.print("session: " + String(session));
        if (String(session) != "" && isSessionValid(session)) {
          if (isTokenValid(session)) {
            Serial.println("Authentification Success");
            return true;
          } else {
            refreshActiveToken();
            if (isTokenValid(session)) {
              Serial.println("Authentification Success");
              return true;
            }
          }
        }
      } else {

        Serial.println("Cookie does not exist: " + this->server->header("Cookie"));
      }
      Serial.println("Authentification Failed");
      this->redirectToLoginPage();
      return false;
    }




    void setup(ESP8266WebServer* server) {
      this->server = server;
      const char * headerkeys[] = {"User-Agent", "Cookie"} ;
      size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
      //ask server to track these headers
      this->server->collectHeaders(headerkeys, headerkeyssize);

      Keycloak* k = this;
      this->server->on ( "/callback", [this]() {
        this->callBackHandle();
      });
    }

};
#endif /* keycloak_h */
