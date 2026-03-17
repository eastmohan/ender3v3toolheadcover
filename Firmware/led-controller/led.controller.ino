#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>


#define LED_PIN     2
#define NUM_LEDS    16
#define BRIGHTNESS  255

CRGB leds[NUM_LEDS];

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

WebServer server(80);

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Printer LED Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; text-align: center; background: #111; color: white; }
    h1 { margin-top: 20px; }
    button, input { margin: 10px; padding: 10px; font-size: 16px; }
  </style>
</head>
<body>
  <h1>LED Control</h1>

  <input type="color" id="colorPicker">

  <br>
  <label>Brightness</label><br>
  <input type="range" id="brightness" min="0" max="255" value="80">

  <br>
  <button onclick="setMode('solid')">Solid</button>
  <button onclick="setMode('rainbow')">Rainbow</button>
  <button onclick="setMode('off')">Off</button>

<script>
function send(path){
  fetch(path);
}

function setMode(mode){
  send('/mode?m=' + mode);
}

document.getElementById("colorPicker").addEventListener("input", function(){
  send('/color?c=' + this.value.substring(1));
});

document.getElementById("brightness").addEventListener("input", function(){
  send('/brightness?b=' + this.value);
});
</script>

</body>
</html>
)rawliteral";

String currentMode = "solid";
CRGB currentColor = CRGB::White;
int currentBrightness = 80;
uint8_t hue = 0;

void applySolid() {
  fill_solid(leds, NUM_LEDS, currentColor);
  FastLED.show();
}

void applyOff() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleColor() {
  if (server.hasArg("c")) {
    String hex = server.arg("c");
    long number = strtol(hex.c_str(), NULL, 16);
    currentColor = CRGB((number >> 16) & 0xFF, (number >> 8) & 0xFF, number & 0xFF);
    currentMode = "solid";
    applySolid();
  }
  server.send(200, "text/plain", "OK");
}

void handleBrightness() {
  if (server.hasArg("b")) {
    currentBrightness = server.arg("b").toInt();
    FastLED.setBrightness(currentBrightness);
    if (currentMode == "solid") applySolid();
  }
  server.send(200, "text/plain", "OK");
}

void handleMode() {
  if (server.hasArg("m")) {
    currentMode = server.arg("m");
    if (currentMode == "off") applyOff();
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(currentBrightness);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/color", handleColor);
  server.on("/brightness", handleBrightness);
  server.on("/mode", handleMode);

  server.begin();
}

void loop() {
  server.handleClient();

  if (currentMode == "rainbow") {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue + i * 10, 255, 255);
    }
    FastLED.show();
    hue++;
    delay(20);
  }
}