#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const int soilMoisturePin = 35;
const int pumpRelayPin = 27;
const int raindropSensorPin = 34;
const int bltin_led = 2;
const int ldr = 32;
int led1 = 26;
int led2=13;
int led3 =14;

const int soilMoistureThreshold = 2500;
const int rainThreshold = 1000;

const char* ssid = "AndroidAP3ADD";
const char* password = "mama007eg";

AsyncWebServer server(80);

int lastSoilMoistureValue = -1;

void setup() {
  pinMode(soilMoisturePin, INPUT);
  pinMode(pumpRelayPin, OUTPUT);
  pinMode(ldr, INPUT);
  pinMode(bltin_led, OUTPUT);
  pinMode(led1, OUTPUT); // Added line for the light sensor
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><head><style>";
    html += "#soilMoisture { font-weight: bold; font-size: 50px; }";
    html += "#rainStatus { font-weight: bold; font-size: 50px; margin-top: 25px; color: black; }";  // Default color is black
    html += "#lightStatus { font-weight: bold; font-size: 50px; margin-top: 25px; }"; // Added styling for light status
    html += "</style></head><body>";
    html += "<h1 style='text-align:center; font-weight:bold; font-size:100px;'>Smart Garden</h1>";
    html += "<p id='soilMoisture'>Soil Moisture Value: </p>";
    html += "<p id='rainStatus'></p>";
    html += "<p id='lightStatus'></p>"; // Added paragraph for light status
    html += "<script>function updateSoilMoisture() {";
    html += "var xhr = new XMLHttpRequest();";
    html += "xhr.onreadystatechange = function() {";
    html += "if (xhr.readyState == 4 && xhr.status == 200) {";
    html += "var value = parseInt(xhr.responseText);";
    html += "var color = value > 2000 ? '#fe7f2d' : '#57cc99';";
    html += "document.getElementById('soilMoisture').innerHTML = '<strong>Soil Moisture Value:</strong> ' + '<span style=\"color:' + color + '\">' + value + '</span>';";
    html += "}";
    html += "};";
    html += "xhr.open('GET', '/soilmoisture', true);";
    html += "xhr.send();";
    html += "}";
    html += "setInterval(updateSoilMoisture, 1000);</script>";
    html += "<script>function updateRainStatus() {";
    html += "var xhr = new XMLHttpRequest();";
    html += "xhr.onreadystatechange = function() {";
    html += "if (xhr.readyState == 4 && xhr.status == 200) {";
    html += "var rainStatus = parseInt(xhr.responseText) < 1000 ? 'It is raining!' : 'No rain ';";
    html += "var color = rainStatus.includes('raining') ? '#5398BE' : 'black';";
    html += "document.getElementById('rainStatus').innerText = rainStatus;";
    html += "document.getElementById('rainStatus').style.color = color;";
    html += "}";
    html += "};";
    html += "xhr.open('GET', '/rainstatus', true);";
    html += "xhr.send();";
    html += "}";
    html += "setInterval(updateRainStatus, 1000);</script>";
    html += "<script>function updateLightStatus() {";
    html += "var xhr = new XMLHttpRequest();";
    html += "xhr.onreadystatechange = function() {";
    html += "if (xhr.readyState == 4 && xhr.status == 200) {";
    html += "var value = parseInt(xhr.responseText);";
    html += "var status = value < 1800 ? 'OFF' : 'ON';";
    html += "var color = value < 1800 ? '#006d77' : '#F2CD5D';";
    html += "document.getElementById('lightStatus').innerHTML = '<strong>Lights Status:</strong> ' + '<span style=\"color:' + color + '\">' + status + '</span>';";
    html += "}";
    html += "};";
    html += "xhr.open('GET', '/lightstatus', true);";
    html += "xhr.send();";
    html += "}";
    html += "setInterval(updateLightStatus, 1000);</script>"; // Added script for light status
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Serve the soil moisture value as plain text
  server.on("/soilmoisture", HTTP_GET, [](AsyncWebServerRequest *request) {
    int soilMoistureValue = analogRead(soilMoisturePin);
    if (soilMoistureValue != lastSoilMoistureValue) {
      lastSoilMoistureValue = soilMoistureValue;
    }
    request->send(200, "text/plain", String(soilMoistureValue));
  });

  // Serve the rain status as plain text
  server.on("/rainstatus", HTTP_GET, [](AsyncWebServerRequest *request) {
    int rainStatus = analogRead(raindropSensorPin);
    request->send(200, "text/plain", String(rainStatus));
  });

  // Serve the light sensor value as plain text
  server.on("/lightstatus", HTTP_GET, [](AsyncWebServerRequest *request) {
    int lightStatus = analogRead(ldr);
    request->send(200, "text/plain", String(lightStatus));
  });

  server.begin();
}

void loop() {
  int soilMoistureValue = analogRead(soilMoisturePin);
  int rainStatus = analogRead(raindropSensorPin);

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoistureValue);

  Serial.print("Rain Status: ");
  Serial.println(rainStatus);

  int ldrValue = analogRead(ldr); // Assign a value to ldrValue

  Serial.print("Light Status: ");
  Serial.println(ldrValue);

  // Check if soil moisture is below the threshold
  if (soilMoistureValue < soilMoistureThreshold) {
    // Turn on the pump by activating the relay
    digitalWrite(pumpRelayPin, HIGH);
    // Serial.println("Water Pump OFF");
  } else {
    // Turn off the pump by deactivating the relay
    digitalWrite(pumpRelayPin, LOW);
    // Serial.println("Water Pump ON");
  }

  // Check if it's raining based on the threshold value
  if (rainStatus < rainThreshold) {
    digitalWrite(pumpRelayPin, HIGH);
  }

  // Check if it's dark based on the LDR reading
  if (ldrValue < 1800) {
    Serial.println("It's bright!");
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
  } else {
    Serial.println("It's dark!");
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
  }

  // Add any additional functionality for the light sensor as needed

  delay(1000); // Delay for stability in serial output
}