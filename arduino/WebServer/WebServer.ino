// WiFi web server with REST API to toggle the top-right matrix LED.
// Status codes on the matrix: 1=boot, 2=WiFi, 3=ready, E=error

#include "WiFiS3.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

const char ssid[] = "foo";
const char pass[] = "foo";

ArduinoLEDMatrix matrix;
WiFiServer server(80);

int wifiStatus = WL_IDLE_STATUS;
bool topRightOn = false;
char statusText[2] = "1";

// Top-right pixel on the 12x8 matrix
const int TOP_RIGHT_X = 11;
const int TOP_RIGHT_Y = 0;

const char indexHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Matrix Light</title>
<script src="https://unpkg.com/vue@3/dist/vue.global.prod.js"></script>
<style>
*{box-sizing:border-box}body{font-family:system-ui,sans-serif;margin:0;padding:2rem;background:#111;color:#eee;text-align:center}
h1{font-size:1.4rem;margin:0 0 .5rem}p{color:#aaa;margin:0 0 1.5rem}
button{font-size:1rem;padding:.75rem 1.5rem;border:0;border-radius:6px;cursor:pointer;color:#fff}
button.on{background:#2e7d32}button.off{background:#555}
button:disabled{opacity:.5;cursor:wait}
</style>
</head>
<body>
<div id="app">
<h1>Matrix Light</h1>
<p>Top-right LED: <strong>{{ on ? 'ON' : 'OFF' }}</strong></p>
<button :class="on?'on':'off'" :disabled="busy" @click="toggle">{{ on ? 'Turn Off' : 'Turn On' }}</button>
</div>
<script>
const{createApp}=Vue
createApp({
  data(){return{on:false,busy:false}},
  mounted(){this.refresh()},
  methods:{
    async refresh(){
      const r=await fetch('/api/light')
      const d=await r.json()
      this.on=!!d.on
    },
    async toggle(){
      this.busy=true
      try{
        const r=await fetch('/api/light/toggle',{method:'POST'})
        const d=await r.json()
        this.on=!!d.on
      }finally{this.busy=false}
    }
  }
}).mount('#app')
</script>
</body>
</html>
)rawliteral";

void refreshMatrix() {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.text(statusText, 3, 1);
  matrix.set(TOP_RIGHT_X, TOP_RIGHT_Y, topRightOn ? 255 : 0, topRightOn ? 255 : 0,
             topRightOn ? 255 : 0);
  matrix.endDraw();
}

void setStatus(char code) {
  statusText[0] = code;
  statusText[1] = '\0';
  refreshMatrix();
}

void sendJson(WiFiClient& client, int code, const char* json) {
  client.print(F("HTTP/1.1 "));
  client.print(code);
  client.println(code == 200 ? F(" OK") : F(" Not Found"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));
  client.println(F("Access-Control-Allow-Methods: GET, POST, OPTIONS"));
  client.println();
  client.println(json);
}

void sendHtml(WiFiClient& client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html; charset=utf-8"));
  client.println(F("Connection: close"));
  client.println();
  for (unsigned int i = 0; i < strlen_P(indexHtml); i++) {
    client.write(pgm_read_byte(indexHtml + i));
  }
}

void sendNotFound(WiFiClient& client) {
  sendJson(client, 404, "{\"error\":\"not found\"}");
}

void printWifiStatus() {
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("RSSI: "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));
}

bool toggleTopRight() {
  topRightOn = !topRightOn;
  refreshMatrix();
  return topRightOn;
}

void handleRequest(WiFiClient& client, const String& requestLine) {
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

  if (method == F("GET") && path == F("/api/light")) {
    char json[32];
    snprintf(json, sizeof(json), "{\"on\":%s}", topRightOn ? "true" : "false");
    sendJson(client, 200, json);
    return;
  }

  if (method == F("POST") && path == F("/api/light/toggle")) {
    toggleTopRight();
    char json[32];
    snprintf(json, sizeof(json), "{\"on\":%s}", topRightOn ? "true" : "false");
    sendJson(client, 200, json);
    return;
  }

  if (method == F("OPTIONS")) {
    client.println(F("HTTP/1.1 204 No Content"));
    client.println(F("Access-Control-Allow-Methods: GET, POST, OPTIONS"));
    client.println(F("Connection: close"));
    client.println();
    return;
  }

  sendNotFound(client);
}

void setup() {
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

  // WL_CONNECTED means associated with the AP; DHCP may still be in progress.
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

void loop() {
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
