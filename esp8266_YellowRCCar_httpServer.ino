#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>

//Here, pins controlling the rear wheel motor
#define In1 D5
#define In2 D6
//Bellow this, I define pins controlling motor turning the car left/right
#define In3 D7
#define In4 D8

#define traveledForward 1
#define traveledBackward 2
#define turnedLeft 3
#define turnedRight 4

const char* ssid = "ESPNet";
const char* password = "_k430MNLikhESPNet_";
ESP8266WebServer server(80);
volatile static uint8_t motionCode = 0;

void goForward();
void goBackward();
void turnRight();
void turnLeft();
void stopEngines();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(In1, OUTPUT);
  pinMode(In2, OUTPUT);
  pinMode(In3, OUTPUT);
  pinMode(In4, OUTPUT);
  digitalWrite(In1, LOW);
  digitalWrite(In2, LOW);
  digitalWrite(In3, LOW);
  digitalWrite(In4, LOW);
  delay(10);
  Serial.println('\n');
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ... ");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  SPIFFS.begin();

  server.on("/FORWARD", HTTP_GET, goForward);
  server.on("/BACKWARD", HTTP_GET, goBackward);
  server.on("/LEFT", HTTP_GET, turnLeft);
  server.on("/RIGHT", HTTP_GET, turnRight);
  server.on("/STOP", HTTP_GET, stopEngines);
  server.on("/PWMDATA", HTTP_POST, setPWM);
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: not found");
  });
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
}

String getContentType(String filename)
{
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpeg";

  return "text/plain";
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  else {
    Serial.println("\tFile not found");
    return false;
  }
}

void goForward() {
  digitalWrite(In1, HIGH);
  digitalWrite(In2, LOW);
  server.sendHeader("Location", "/Forward.html");
  server.send(303);
  motionCode = traveledForward;
}

void goBackward() {
  digitalWrite(In1, LOW);
  digitalWrite(In2, HIGH);
  server.sendHeader("Location", "/Backward.html");
  server.send(303);
  motionCode = traveledBackward;
}

void turnLeft() {
  digitalWrite(In3, HIGH);
  digitalWrite(In4, LOW);
  server.sendHeader("Location", "/Left.html");
  server.send(303);
  motionCode = turnedLeft;
}

void turnRight() {
  digitalWrite(In3, LOW);
  digitalWrite(In4, HIGH);
  server.sendHeader("Location", "/Right.html");
  server.send(303);
  motionCode = turnedRight;
}

void stopEngines() {
  digitalWrite(In1, LOW);
  digitalWrite(In2, LOW);
  digitalWrite(In3, LOW);
  digitalWrite(In4, LOW);

  switch (motionCode) {
    case traveledForward:
      server.sendHeader("Location", "Forward.html");
      server.send(303);
      break;
    case traveledBackward:
      server.sendHeader("Location", "Backward.html");
      server.send(303);
      break;
    case turnedLeft:
      server.sendHeader("Location", "Left.html");
      server.send(303);
      break;
    case turnedRight:
      server.sendHeader("Location", "Right.html");
      server.send(303);
      break;
    default:
      server.sendHeader("Location", "index.html");
      server.send(303);
      break;
  }
}

void setPWM() {
  if (server.hasArg("plain") == false) { //Check if body received

    server.send(200, "text/plain", "Body not received");
    return;
  }

  String message = server.arg("plain");
  int pwmValue = message.toInt();
  pwmValue = map(pwmValue,0,100,0,1023);
  
  server.send(200, "text/plain", message);
  Serial.println(pwmValue);
}

