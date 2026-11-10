#include "WiFiS3.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

char ssid[] = "R4-Test";
char pass[] = "12345678";

int status = WL_IDLE_STATUS;
WiFiServer server(80);
ArduinoLEDMatrix matrix;

int currentNumber = 0;
unsigned long lastUpdate = 0;
const unsigned long interval = 10000;  // 10 seconds

void setup() {
  Serial.begin(9600);
  matrix.begin();

  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("AP creation failed");
    while (true);
  }

  delay(1000);
  server.begin();
  Serial.print("Hotspot IP: ");
  Serial.println(WiFi.localIP());

  currentNumber = random(0, 1000);
  showOnMatrix(currentNumber);
  lastUpdate = millis();
}

void loop() {
  // Every 10 seconds: new number, update LED matrix
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();
    currentNumber = random(0, 1000);
    showOnMatrix(currentNumber);
  }

  // Serve the webpage
  WiFiClient client = server.available();
  if (client) {
    client.println("<!DOCTYPE HTML>");
client.println("<html>");
client.println("<head>");
client.println("<meta charset='UTF-8'>");
client.println("<title>R4 Random Number</title>");
client.println("</head>");
client.println("<body>");
client.println("<h1>Random Number:</h1>");
client.println("<p style='font-size:48px;'>" + String(currentNumber) + "</p>");
client.println("<p>This page will refresh every 5 seconds</p>");
client.println("<script>");
client.println("setTimeout(function(){ location.reload(); }, 5000);");
client.println("</script>");
client.println("</body>");
client.println("</html>");

    delay(1);
    client.stop();
  }
}

void showOnMatrix(int value) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(String(value));
  matrix.endText();
  matrix.endDraw();
}