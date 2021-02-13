#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* SSID = "";
const char* PASSWORD = "";

ESP8266WebServer server(80);

const int LED_PIN = 2;
const int RED_PIN = 15; // D8
const int GREEN_PIN = 12; // D6
const int BLUE_PIN = 13; // D7
const int LDR_PIN = A0;

const int STATUS_AUTO = 2;
const int STATUS_ON = 1;
const int STATUS_OFF = 0;



String stripColor="ffffff";
int statusMode = STATUS_AUTO;
bool stripStatus = false;
bool lightSensorStatus = false;
int lightTriggerValue = 900;
int lightSensorValue = 0;


void setup(void) {
  pinMode(LED_PIN, OUTPUT);  
  pinMode(RED_PIN,OUTPUT);
  pinMode(GREEN_PIN,OUTPUT);
  pinMode(BLUE_PIN,OUTPUT);
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  // By default auto
  setStripStatus(STATUS_AUTO);  

  server.on("/", handleRoot);

  server.on("/status", []() {
    returnStatus();
  });

  server.on("/on", []() {    
    setStatusMode(STATUS_ON);    
    returnStatus();
  });

  server.on("/off", []() {
    setStatusMode(STATUS_OFF);        
    returnStatus();
  });

  server.on("/auto", []() {
    setStatusMode(STATUS_AUTO);    
    returnStatus();
  });

  server.on("/color", []() {
    String color = server.arg("color");
    setStripColor(color);
    returnStatus();
  });

  server.on("/light-trigger", []() {    
    lightTriggerValue = atoi(server.arg("value").c_str());
    returnStatus();
  });
  

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}


void handleRoot() {  
  server.send(200, "text/plain", "hello from led strip!\r\n");  
}

void updateStrip(void) {
  long number = (int) strtol( &stripColor[0], NULL, 16);
  //Split them up into r, g, b values
  long r = number >> 16;
  long g = (number >> 8) & 0xFF;
  long b = number & 0xFF;

  //PWM Correction
  r = r * 4; 
  g = g * 4;
  b = b * 4;

  if (statusMode == STATUS_ON || (statusMode == STATUS_AUTO && lightSensorStatus)) {
    analogWrite(RED_PIN,r);
    analogWrite(GREEN_PIN,g);
    analogWrite(BLUE_PIN,b);
    setStripStatus(true);
  } else {
    analogWrite(RED_PIN,0);
    analogWrite(GREEN_PIN,0);
    analogWrite(BLUE_PIN,0);
    setStripStatus(true);
  }
}

void setStatusMode(int mode) {
  statusMode = mode;
}

void setStripStatus(bool newStatus) {
  digitalWrite(LED_PIN, newStatus ? LOW : HIGH);  
  stripStatus = newStatus;
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);  
}

void returnStatus(void) {
  String res = "{ mode: \"";
  switch (stripStatus) {
    default:
    case STATUS_OFF:
      res += "on";
      break;
    case STATUS_ON:
      res += "off";
      break;
    case STATUS_AUTO:
      res += "auto";
      break;      
  }
  res += "\", ";
  
  char statusStr[255];
  sprintf(statusStr, "status: %s", stripStatus ? "true" : "false"); 
  res += statusStr;
  res += ", ";

  char colorStr[255];
  sprintf(colorStr, "color: \"#%s\"", stripColor.c_str());
  res += colorStr;
  res += ", ";

  char valueStr[255];
  sprintf(valueStr, "sensor-value: %d", lightSensorValue);      
  res += valueStr;
  res += ", ";

  char sensorStatusStr[255];
  sprintf(sensorStatusStr, "sensor-status: %s", lightSensorStatus ? "true" : "false");      
  res += sensorStatusStr;
  res += ", ";

  char lightTrigerValueStr[255];
  sprintf(lightTrigerValueStr, "light-trigger-value: %d", lightTriggerValue);      
  res += lightTrigerValueStr;

  
  res += " }";
  server.send(200, "application/json", res);
}

void setStripColor(String color) {
  stripColor = color;    
}

void loop(void) {
  lightSensorValue = analogRead(LDR_PIN);

  if (lightSensorValue < lightTriggerValue) {
    lightSensorStatus = true;
  } else {
    lightSensorStatus = false;
  }

  updateStrip();
  delay(500);
    
  server.handleClient();
  MDNS.update();
}
