#include "web_server.h"
#include "strip_api.h"

#include "WiFiS3.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

const char ssid[] = "foo";
const char pass[] = "foo";

ArduinoLEDMatrix matrix;
WiFiServer server(80);

int wifiStatus = WL_IDLE_STATUS;
char statusText[2] = "1";

static void refreshMatrix() {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.text(statusText, 3, 1);
  matrix.endDraw();
}

static void setStatus(char code) {
  statusText[0] = code;
  statusText[1] = '\0';
  refreshMatrix();
}

static void sendJson(WiFiClient& client, int code, const char* json) {
  client.print(F("HTTP/1.1 "));
  client.print(code);
  client.println(code == 200 ? F(" OK") : F(" Not Found"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F("Access-Control-Allow-Methods: GET, POST, OPTIONS"));
  client.println(F("Access-Control-Allow-Headers: Content-Type"));
  client.println();
  client.println(json);
}

static void sendHtml(WiFiClient& client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html; charset=utf-8"));
  client.println(F("Connection: close"));
  client.println();

  IPAddress ip = WiFi.localIP();
  char ipStr[16];
  snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  const char htmlPrefix[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Arduino Lights - API Server</title>
<style>
*{box-sizing:border-box}
body{font-family:system-ui,-apple-system,sans-serif;margin:0;padding:2rem 1rem;background:#111;color:#eee;text-align:center;min-height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center}
h1{font-size:1.4rem;margin:0 0 .5rem;font-weight:600}
p{color:#aaa;margin:0 0 .25rem;line-height:1.5}
code{background:#2a2a2a;padding:.15em .5em;border-radius:4px;font-size:.9em;color:#eee}
.ip{font-size:1.5rem;margin:.75rem 0 1.5rem}
a{color:#4caf50}
footer{margin-top:2.5rem;font-size:.8rem;color:#555}
</style>
</head>
<body>
<h1>Arduino Lights</h1>
<p>API server is running</p>
<p class="ip"><code>)rawliteral";

  const char htmlSuffix[] PROGMEM = R"rawliteral(
</code></p>
<p>Use the <a href="https://nolanpro.github.io/arduino-lights/" target="_blank">web controller</a> to control the LEDs.</p>
<footer>StripLightsTest &middot; Arduino Uno R4 WiFi</footer>
</body>
</html>
)rawliteral";

  for (unsigned int i = 0; i < strlen_P(htmlPrefix); i++) {
    client.write(pgm_read_byte(htmlPrefix + i));
  }
  client.print(ipStr);
  for (unsigned int i = 0; i < strlen_P(htmlSuffix); i++) {
    client.write(pgm_read_byte(htmlSuffix + i));
  }
}

static void sendNotFound(WiFiClient& client) {
  sendJson(client, 404, "{\"error\":\"not found\"}");
}

static void printWifiStatus() {
  Serial.println(F("WiFi Firmware Version: "));
  Serial.println(WiFi.firmwareVersion());
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("RSSI: "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));
}

static void handleRequest(WiFiClient& client, const String& requestLine) {
  String method;
  String path;

  int firstSpace = requestLine.indexOf(' ');
  if (firstSpace >= 0) {
    method = requestLine.substring(0, firstSpace);
    int secondSpace = requestLine.indexOf(' ', firstSpace + 1);
    path = secondSpace >= 0 ? requestLine.substring(firstSpace + 1, secondSpace)
                            : requestLine.substring(firstSpace + 1);
  }

  if (method == F("GET") &&
      (path == F("/") || path.startsWith(F("/?")) || path == F("/index.html"))) {
    sendHtml(client);
    return;
  }

  if (method == F("GET") && path == F("/api/strip")) {
    char json[48];
    snprintf(json, sizeof(json), "{\"on\":%s,\"brightness\":%d}", is_strip_on() ? "true" : "false", strip_get_brightness());
    sendJson(client, 200, json);
    return;
  }

  if (method == F("POST") && path == F("/api/strip/toggle")) {
    strip_toggle();
    char json[32];
    snprintf(json, sizeof(json), "{\"on\":%s}", is_strip_on() ? "true" : "false");
    sendJson(client, 200, json);
    return;
  }

  if (method == F("GET") && path == F("/api/strip/scene")) {
    char json[96];
    snprintf(json, sizeof(json), "{\"scene\":\"%s\"}", strip_get_scene());
    sendJson(client, 200, json);
    return;
  }

  if (method == F("GET") && path == F("/api/strip/scenes")) {
    sendJson(client, 200, "{\"scenes\":[\"blue-noise\",\"rainbow-chase\",\"single-led\",\"solid-color\",\"rainbow\",\"rainbow-with-glitter\",\"confetti\",\"sinelon\",\"juggle\",\"bpm\"]}");
    return;
  }

  // POST /api/strip/scene/<name>
  if (method == F("POST") && path.startsWith(F("/api/strip/scene/"))) {
    String sceneName = path.substring(17);  // after "/api/strip/scene/"
    if (sceneName == F("blue-noise") || sceneName == F("rainbow-chase") || sceneName == F("single-led") || sceneName == F("solid-color") || sceneName == F("rainbow") || sceneName == F("rainbow-with-glitter") || sceneName == F("confetti") || sceneName == F("sinelon") || sceneName == F("juggle") || sceneName == F("bpm")) {
      char buf[32];
      sceneName.toCharArray(buf, sizeof(buf));
      strip_set_scene(buf);
      char json[96];
      snprintf(json, sizeof(json), "{\"scene\":\"%s\"}", strip_get_scene());
      sendJson(client, 200, json);
    } else {
      sendNotFound(client);
    }
    return;
  }

  // POST /api/strip/pixel/<index>
  if (method == F("POST") && path.startsWith(F("/api/strip/pixel/"))) {
    String indexStr = path.substring(17);  // after "/api/strip/pixel/"
    int index = indexStr.toInt();
    strip_set_single_led_pixel(index);
    strip_set_scene("single-led");
    char json[64];
    snprintf(json, sizeof(json), "{\"index\":%d,\"scene\":\"single-led\"}", index);
    sendJson(client, 200, json);
    return;
  }

  // GET /api/strip/brightness
  if (method == F("GET") && path == F("/api/strip/brightness")) {
    char json[32];
    snprintf(json, sizeof(json), "{\"brightness\":%d}", strip_get_brightness());
    sendJson(client, 200, json);
    return;
  }

  // POST /api/strip/brightness
  if (method == F("POST") && path == F("/api/strip/brightness")) {
    // Read the request body for JSON
    String body = "";
    while (client.available()) {
      char c = client.read();
      body += c;
    }
    // Simple JSON parse: find "brightness":<number>
    int keyIdx = body.indexOf(F("\"brightness\""));
    if (keyIdx >= 0) {
      int colonIdx = body.indexOf(':', keyIdx);
      if (colonIdx >= 0) {
        // Skip whitespace after colon
        int numStart = colonIdx + 1;
        while (numStart < (int)body.length() && body[numStart] == ' ') numStart++;
        int numEnd = numStart;
        while (numEnd < (int)body.length() && body[numEnd] >= '0' && body[numEnd] <= '9') numEnd++;
        if (numEnd > numStart) {
          String numStr = body.substring(numStart, numEnd);
          int val = numStr.toInt();
          if (val < 0) val = 0;
          if (val > 255) val = 255;
          strip_set_brightness((uint8_t)val);
          char json[32];
          snprintf(json, sizeof(json), "{\"brightness\":%d}", strip_get_brightness());
          sendJson(client, 200, json);
          return;
        }
      }
    }
    sendJson(client, 200, "{\"error\":\"invalid brightness\"}");
    return;
  }

  // GET /api/strip/hue
  if (method == F("GET") && path == F("/api/strip/hue")) {
    char json[32];
    snprintf(json, sizeof(json), "{\"hue\":%d}", strip_get_hue());
    sendJson(client, 200, json);
    return;
  }

  // POST /api/strip/hue
  if (method == F("POST") && path == F("/api/strip/hue")) {
    String body = "";
    while (client.available()) {
      char c = client.read();
      body += c;
    }
    int keyIdx = body.indexOf(F("\"hue\""));
    if (keyIdx >= 0) {
      int colonIdx = body.indexOf(':', keyIdx);
      if (colonIdx >= 0) {
        int numStart = colonIdx + 1;
        while (numStart < (int)body.length() && body[numStart] == ' ') numStart++;
        int numEnd = numStart;
        while (numEnd < (int)body.length() && body[numEnd] >= '0' && body[numEnd] <= '9') numEnd++;
        if (numEnd > numStart) {
          String numStr = body.substring(numStart, numEnd);
          int val = numStr.toInt();
          if (val < 0) val = 0;
          if (val > 359) val = 359;
          strip_set_hue((uint8_t)val);
          char json[32];
          snprintf(json, sizeof(json), "{\"hue\":%d}", strip_get_hue());
          sendJson(client, 200, json);
          return;
        }
      }
    }
    sendJson(client, 200, "{\"error\":\"invalid hue\"}");
    return;
  }

  // GET /api/strip/color
  if (method == F("GET") && path == F("/api/strip/color")) {
    uint8_t r, g, b;
    strip_get_color(&r, &g, &b);
    char json[48];
    snprintf(json, sizeof(json), "{\"r\":%d,\"g\":%d,\"b\":%d}", r, g, b);
    sendJson(client, 200, json);
    return;
  }

  // POST /api/strip/color
  if (method == F("POST") && path == F("/api/strip/color")) {
    String body = "";
    while (client.available()) {
      char c = client.read();
      body += c;
    }
    int r = -1, g = -1, b = -1;
    int rIdx = body.indexOf(F("\"r\""));
    int gIdx = body.indexOf(F("\"g\""));
    int bIdx = body.indexOf(F("\"b\""));
    if (rIdx >= 0) {
      int colonIdx = body.indexOf(':', rIdx);
      if (colonIdx >= 0) {
        int numStart = colonIdx + 1;
        while (numStart < (int)body.length() && body[numStart] == ' ') numStart++;
        int numEnd = numStart;
        while (numEnd < (int)body.length() && body[numEnd] >= '0' && body[numEnd] <= '9') numEnd++;
        if (numEnd > numStart) {
          r = constrain(body.substring(numStart, numEnd).toInt(), 0, 255);
        }
      }
    }
    if (gIdx >= 0) {
      int colonIdx = body.indexOf(':', gIdx);
      if (colonIdx >= 0) {
        int numStart = colonIdx + 1;
        while (numStart < (int)body.length() && body[numStart] == ' ') numStart++;
        int numEnd = numStart;
        while (numEnd < (int)body.length() && body[numEnd] >= '0' && body[numEnd] <= '9') numEnd++;
        if (numEnd > numStart) {
          g = constrain(body.substring(numStart, numEnd).toInt(), 0, 255);
        }
      }
    }
    if (bIdx >= 0) {
      int colonIdx = body.indexOf(':', bIdx);
      if (colonIdx >= 0) {
        int numStart = colonIdx + 1;
        while (numStart < (int)body.length() && body[numStart] == ' ') numStart++;
        int numEnd = numStart;
        while (numEnd < (int)body.length() && body[numEnd] >= '0' && body[numEnd] <= '9') numEnd++;
        if (numEnd > numStart) {
          b = constrain(body.substring(numStart, numEnd).toInt(), 0, 255);
        }
      }
    }
    if (r >= 0 && g >= 0 && b >= 0) {
      strip_set_color((uint8_t)r, (uint8_t)g, (uint8_t)b);
      char json[48];
      snprintf(json, sizeof(json), "{\"r\":%d,\"g\":%d,\"b\":%d}", r, g, b);
      sendJson(client, 200, json);
      return;
    }
    sendJson(client, 200, "{\"error\":\"invalid color\"}");
    return;
  }

  if (method == F("OPTIONS")) {
    client.println(F("HTTP/1.1 204 No Content"));
    client.println(F("Access-Control-Allow-Origin: *"));
    client.println(F("Access-Control-Allow-Methods: GET, POST, OPTIONS"));
    client.println(F("Access-Control-Allow-Headers: Content-Type"));
    client.println(F("Connection: close"));
    client.println();
    return;
  }

  sendNotFound(client);
}

void web_setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  matrix.begin();
  setStatus('1');

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("WiFi module not found"));
    setStatus('E');
    while (true) {
      delay(1000);
    }
  }

  Serial.print(F("Firmware: "));
  Serial.println(WiFi.firmwareVersion());

  setStatus('2');
  WiFi.setHostname("Arduino");
  while (wifiStatus != WL_CONNECTED) {
    Serial.print(F("Connecting to "));
    Serial.println(ssid);
    wifiStatus = WiFi.begin(ssid, pass);
    if (wifiStatus != WL_CONNECTED) {
      delay(2000);
    }
  }

  Serial.print(F("Waiting for IP"));
  unsigned long dhcpStart = millis();
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    if (millis() - dhcpStart > 30000) {
      Serial.println(F("\nDHCP failed"));
      setStatus('E');
      while (true) {
        delay(1000);
      }
    }
    delay(500);
    Serial.print('.');
  }
  Serial.println();

  printWifiStatus();
  server.begin();
  setStatus('3');
  Serial.println(F("Web server ready"));
}

void web_loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println(F("Client connected"));
  String currentLine = "";
  String requestLine = "";

  while (client.connected()) {
    if (!client.available()) {
      continue;
    }

    char c = client.read();
    if (c == '\n') {
      if (currentLine.length() == 0) {
        handleRequest(client, requestLine);
        break;
      }

      if (requestLine.length() == 0 &&
          (currentLine.startsWith(F("GET ")) || currentLine.startsWith(F("POST ")) ||
           currentLine.startsWith(F("OPTIONS ")))) {
        requestLine = currentLine;
      }
      currentLine = "";
    } else if (c != '\r') {
      currentLine += c;
    }
  }

  delay(1);
  client.stop();
  Serial.println(F("Client disconnected"));
}