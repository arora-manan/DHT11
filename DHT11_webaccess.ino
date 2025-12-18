#include <Adafruit_Sensor.h>

#include <DHT.h>
#include <DHT_U.h>

 #include <ESP8266WiFi.h>
 #include <ESP8266WebServer.h>
 #include <ESP8266mDNS.h>


#define DHTPIN D4         // GPIO2 on NodeMCU (D4)
#define DHTTYPE DHT11     // or DHT22

DHT dht(DHTPIN, DHTTYPE);
// Replace with SSID and PASSWORD for the ESP32 ACCESS POINT
// (for testing you can leave the default)
const char* ssid = "ESP8266_ACCESS_POINT";
const char* password = "pass123456";

ESP8266WebServer server(80);

void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<link rel=\"icon\" href=\"data:,\">";
  html += "<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}";
  html += "table { border-collapse: collapse; width:60%; margin-left:auto; margin-right:auto; }";
  html += "th { padding: 10px; background-color: #0043af; color: white; }";
  html += "tr { border: 1px solid #ddd; padding: 10px; }";
  html += "tr:hover { background-color: #bcbcbc; }";
  html += "td { border: none; padding: 8px; }";
  html += ".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }</style></head>";
  html += "<body><h1>ESP8266 with DHT</h1>";
  html += "<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>";
  html += "<tr><td>Temp. Celsius</td><td><span class=\"sensor\">";
  html += String(dht.readTemperature());
  html += " *C</span></td></tr>";
  html += "<tr><td>Humidity</td><td><span class=\"sensor\">";
  html += String(dht.readHumidity());
  html += " %</span></td></tr></table></body></html>";

  // Send the response to the client
  server.send(200, "text/html", html);
}
void setup() {
  Serial.begin(115200);

  dht.begin();
  // Set the ESP32 as access point
  Serial.print("Setting as access point ");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("");
  Serial.println("ESP32 Wi-Fi Access Point ready!");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  if (MDNS.begin("manan")) { Serial.println("MDNS responder started");}

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  MDNS.update();
}



