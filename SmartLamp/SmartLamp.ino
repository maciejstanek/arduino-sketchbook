#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define JSON_BUFFER_SIZE 200
#define SERIAL_BUFFER_SIZE (JSON_BUFFER_SIZE - 20)
#define EEPROM_SIZE 4096
#define SSID_MAX_SIZE 32
#define SSID_EEPROM_ADDR 32
#define PASSWORD_MAX_SIZE 64
#define PASSWORD_EEPROM_ADDR (SSID_EEPROM_ADDR + SSID_MAX_SIZE)

// TODO: Consider configuring the IP via UART/JSON.
IPAddress ip(192, 168, 0, 9);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
String ssid;
String password;
String serialBuffer;
int serialBufferIndex = 0;

ESP8266WebServer server(80);

bool lightEnabled;
bool requestSuccessful;
const byte relayPin = 2;

String generateJsonResponse() {
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& jsonRoot = jsonBuffer.createObject();
  jsonRoot["light"] = lightEnabled ? "on" : "off";
  jsonRoot["success"] = requestSuccessful;
  String jsonResponse;
  jsonRoot.printTo(jsonResponse);
  return jsonResponse;
}
void sendJsonResponse(int statusCode) {
  server.send(statusCode, "application/json", generateJsonResponse());
}

void handleRequestStatus() {
  requestSuccessful = true;
  sendJsonResponse(200);
}
void handleSetOn() {
  lightEnabled = true;
  requestSuccessful = true;
  sendJsonResponse(200);
}
void handleSetOff() {
  lightEnabled = false;
  requestSuccessful = true;
  sendJsonResponse(200);
}
void handleNotFound() {
  requestSuccessful = false;
  sendJsonResponse(404);
}
void handleRequestIndexPage() {
  requestSuccessful = true;
  String html = "";
  html += "<html lang='en'>\n";
  html += "  <head>\n";
  html += "    <meta charset='utf-8'/>\n";
  html += "    <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0'/>\n";
  html += "    <meta name='theme-color' content='black'>\n";
  html += "    <link href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH4QoaFBIZY2geYgAAAAh0RVh0Q29tbWVudAD2zJa/AAABP0lEQVQ4y6WTMWvCUBSFvwSyPUov5i+4uGW2U3DVsWDj6A8R/AHi4OaYROiom2R0DdlcQv9BJG3pluF1aCKvIhraA294l3MO917OtbiAiChgCEyAp7p8AEJgl1TVl8m3LsRjIOY2XpKq2jQf2xDPG7Hn9VitZuR5Qp4nrFYzPK/XUOOB48wv2x6LiHZdV4fhQmv9dvWF4UK7rqtFRIvIOFXqZ+a6cFNsmjR8EVF2vTA8r0cQjLiHIBiZ4wztettMp8+0hcGdWCLyATzkeUKn89jK4HR6p9sdAHza/BN2HRL2+0NrkcE92HXCWK9fWxsY3NAGdgBZdiSKtnfFUbQly47Nd0eq1J+DBECqVGMybwLi+30dx0tdFKkuilTH8VL7ft8M0DnKVqrUub2B47Q6prIsN1cNapOb51yW5a9z/gbt0NOXmSDJtAAAAABJRU5ErkJggg==' rel='icon' type='image/x-icon'>";
  html += "    <title>Smart Lamp UI</title>\n";
  html += "    <style>\n";
  html += "      body {\n";
  html += "        background-color: black;\n";
  html += "        color: white;\n";
  html += "      }\n";
  html += "      #lamp {\n";
  html += "        width: 50%;\n";
  html += "        max-width: 300px;\n";
  html += "        border-radius: 50%;\n";
  html += "        position: absolute;\n";
  html += "        left: 50%;\n";
  html += "        top: 50%;\n";
  html += "        transform: translate(-50%, -50%);\n";
  html += "        transition: background-color 0.1s, box-shadow 0.1s;\n";
  html += "      }\n";
  html += "      #lamp:after {\n";
  html += "        content: '';\n";
  html += "        padding-bottom: 100%;\n";
  html += "        display: block;\n";
  html += "      }\n";
  html += "      .lamp-on {\n";
  html += "        background-color: #FFD;\n";
  html += "        box-shadow: 0 0 70px #FFD;\n";
  html += "      }\n";
  html += "      .lamp-off {\n";
  html += "        background-color: #111;\n";
  html += "      }\n";
  html += "      #author {\n";
  html += "        position: absolute;\n";
  html += "        right: 0;\n";
  html += "        bottom: 0;\n";
  html += "        margin-bottom: 4px;\n";
  html += "        margin-right: 4px;\n";
  html += "        color: #333;\n";
  html += "        font-size: small;\n";
  html += "        font-family: Tahoma, Geneva, sans-serif;\n";
  html += "      }\n";
  html += "    </style>\n";
  html += "  </head>\n";
  html += "  <body>\n";
  html += "    <div id='lamp' class='lamp-off'></div>\n";
  html += "    <div id='author'>&#169; Maciej Stanek 2017</div>\n";
  html += "    <script>\n";
  html += "      (function() {\n";
  html += "        'use strict';\n";
  html += "        var IP = '" + WiFi.localIP().toString() + "';\n";
  html += "        var lamp = document.getElementById('lamp');\n";
  html += "        var lightEnabled = false;\n";
  html += "        var request = new XMLHttpRequest();\n";
  html += "        request.timeout = 4000;\n";
  html += "        request.onreadystatechange = function() {\n";
  html += "          if(request.readyState === 4) {\n";
  html += "            var reponse = JSON.parse(request.responseText);\n";
  html += "            lightEnabled = (reponse['light'] === 'on');\n";
  html += "            lamp.className = lightEnabled ? 'lamp-on' : 'lamp-off';\n";
  html += "          }\n";
  html += "        }\n";
  html += "        function requestLightToggle() {\n";
  html += "          var URL = 'http://' + IP;\n";
  html += "          if(lightEnabled) {\n";
  html += "            URL += '/off';\n";
  html += "          } else {\n";
  html += "            URL += '/on';\n";
  html += "          }\n";
  html += "          request.open('Get', URL);\n";
  html += "          request.send();\n";
  html += "        }\n";
  html += "        function requestLightStatus() {\n";
  html += "          request.open('Get', 'http://' + IP + '/info');\n";
  html += "          request.send();\n";
  html += "        }\n";
  html += "        var intervalHandle = setInterval(requestLightStatus, 1000);\n";
  html += "        window.onclick = requestLightToggle;\n";
  html += "        requestLightStatus();\n";
  html += "      })();\n";
  html += "    </script>\n";
  html += "  </body>\n";
  html += "</html>\n";
  server.send(200, "text/html", html);
}

void updateLightRelay() {
  // NOTE: 3V3 is too low to set relay to 'high' state.
  //       Luckily, this relay module is 'pull-low-to-trigger'
  //       with 5V pull-up. As a result we can force 0V
  //       to turn the relay off and set Z (high impedance
  //       state) to turn it on.
  if(lightEnabled) {
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);
  } else {
    pinMode(relayPin, INPUT);
  }
}

void initLightRelay() {
  lightEnabled = true;
  updateLightRelay();
}

void writeToEEPROM(String string, int address, int maxLength) {
  for(int i = 0; i < maxLength; i++) {
    if(i >= string.length()) {
      EEPROM.write(address + i, 0);
      break;
    }
    EEPROM.write(address + i, string[i]);
  }
}

String readFromEEPROM(int address, int maxLength) {
  String string = "";
  for(int i = 0; i < maxLength; i++) {
    char c = EEPROM.read(address + i);
    if(!c) {
      break;
    }
    string += c;
  }
  return string;
}

void readCredentialsFromEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  ssid = readFromEEPROM(SSID_EEPROM_ADDR, SSID_MAX_SIZE);
  password = readFromEEPROM(PASSWORD_EEPROM_ADDR, PASSWORD_MAX_SIZE);
  EEPROM.end();
}

void saveCredentialsToEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  writeToEEPROM(ssid, SSID_EEPROM_ADDR, SSID_MAX_SIZE);
  writeToEEPROM(password, PASSWORD_EEPROM_ADDR, PASSWORD_MAX_SIZE);
  EEPROM.commit();
  EEPROM.end();
}

void loadAvailableDataFromSerial() {
  while(Serial.available() > 0) {
    if(serialBufferIndex == 0) {
      serialBuffer = "";
    }
    char c = Serial.read();
    if(c == '\n' || ++serialBufferIndex > SERIAL_BUFFER_SIZE) {
      serialBufferIndex = 0;
      continue;
    }
    serialBuffer += c;
  }
}

bool parseSerialBufferAsCredentials() {
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& jsonRoot = jsonBuffer.parseObject(serialBuffer);
  if(!jsonRoot.success()) {
    return false;
  }
  // It is always safe to read even if the keys do not exist.
  ssid = jsonRoot.get<String>("ssid");
  if(ssid.length() == 0) {
    return false;
  }
  password = jsonRoot.get<String>("password");
  if(password.length() == 0) {
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  initLightRelay();

  WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_STA);
  readCredentialsFromEEPROM();
  Serial.print("SSID:     ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  WiFi.begin(ssid.c_str(), password.c_str());
  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print("Cannot connect to '");
    Serial.print(ssid);
    Serial.println("'. Rebooting in 5 seconds.");
    delay(5000);
    loadAvailableDataFromSerial();
    if(parseSerialBufferAsCredentials()) {
      saveCredentialsToEEPROM();
    }
    ESP.restart();
  }
  Serial.print("IP:       ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC:      ");
  Serial.println(WiFi.macAddress());
  ArduinoOTA.begin();

  server.on("/i", handleRequestStatus);
  server.on("/info", handleRequestStatus);
  server.on("/on", handleSetOn);
  server.on("/1", handleSetOn);
  server.on("/off", handleSetOff);
  server.on("/0", handleSetOff);
  server.on("/", handleRequestIndexPage);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  updateLightRelay();
}
