#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_CCS811.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// Configuration
#define DHTPIN 23 
#define DHTTYPE DHT11
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Initialize objects
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_CCS811 ccs;

// WiFi Credentials
const char* ssid = "ESP32_Air_Monitor";
const char* password = "password123";

WebServer server(80);

void handleRoot() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  uint16_t eco2 = 0;
  uint16_t tvoc = 0;

  if (ccs.available() && !ccs.readData()) {
    eco2 = ccs.geteCO2();
    tvoc = ccs.getTVOC();
  }

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<meta http-equiv=\"refresh\" content=\"5\">"; // Auto-refresh every 5s
  html += "<style>body{font-family:sans-serif; text-align:center; background:#f4f4f4;}";
  html += ".card{background:white; padding:20px; border-radius:10px; display:inline-block; margin:10px; box-shadow:0 4px 8px rgba(0,0,0,0.1); width:200px;}";
  html += "h1{color:#0043af;} .val{font-size:1.5em; font-weight:bold; color:#333;}</style></head>";
  html += "<body><h1>ESP32 Environment Monitor</h1>";
  
  html += "<div class='card'>Temp<br><span class='val'>" + String(t, 1) + " °C</span></div>";
  html += "<div class='card'>Humidity<br><span class='val'>" + String(h, 0) + " %</span></div><br>";
  html += "<div class='card'>CO2<br><span class='val'>" + String(eco2) + " ppm</span></div>";
  html += "<div class='card'>TVOC<br><span class='val'>" + String(tvoc) + " ppb</span></div>";
  
  html += "<p>Device: http://manan.local</p></body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // Start OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(10, 20);
  display.setTextSize(1);
  display.print("Starting System...");
  display.display();

  // Start DHT
  dht.begin();

  // Start CCS811
  if (!ccs.begin()) {
    Serial.println("CCS811 failed");
    while(1);
  }

  // Setup WiFi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("AP Started. IP: " + WiFi.softAPIP().toString());

  if (MDNS.begin("manan")) {
    Serial.println("mDNS: http://manan.local");
  }

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();

  // Read Sensors
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (ccs.available() && !ccs.readData()) {
    // Calibrate CCS811 with DHT11 data for better accuracy
    if(!isnan(t) && !isnan(h)) {
      ccs.setEnvironmentalData(h, t);
    }

    uint16_t eco2 = ccs.geteCO2();
    uint16_t tvoc = ccs.getTVOC();

    // Update OLED
    display.clearDisplay();
    display.setTextSize(1);
    
    display.setCursor(0, 0);
    display.printf("CO2:  %d ppm", eco2);
    
    display.setCursor(0, 16);
    display.printf("TVOC: %d ppb", tvoc);
    
    display.setCursor(0, 32);
    display.printf("Temp: %.1f C", t);
    
    display.setCursor(0, 48);
    display.printf("Hum:  %.1f %%", h);
    
    display.display();

    // Serial Print for Debugging
    Serial.printf("CO2: %d | TVOC: %d | T: %.1f | H: %.1f\n", eco2, tvoc, t, h);
  }

  delay(2000); // 2 second update rate
}