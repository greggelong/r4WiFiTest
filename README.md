# UNO R4 WiFi — Wireless Random Number Broadcast (Test)

## Overview

This test proves that an Arduino UNO R4 WiFi can **broadcast data wirelessly with no USB cable connected**. The board creates its own Wi-Fi hotspot, runs a tiny web server, and serves a webpage showing a random number. The same number is displayed on the board's built-in LED matrix, and the number updates automatically every 10 seconds.

This is a proof-of-concept for a wireless sensor pipeline: later, the random number can be replaced with real sensor readings (e.g., an ultrasonic distance sensor).

## Hardware Required

- Arduino UNO R4 WiFi
- USB cable (only needed once, to upload the sketch)
- A phone or computer with Wi-Fi and a web browser
- Optional: USB power bank or phone charger (to power the board after upload)

## How It Works

1. On startup, the board creates a Wi-Fi **Access Point (hotspot)** named `R4-Test` with password `12345678`.
2. The board runs an **HTTP web server** on port 80.
3. A random number is generated **every 10 seconds** on a fixed timer, stored in a variable, and drawn on the LED matrix.
4. When a browser connects to the board's IP address, the board responds with an HTML page showing the current number.
5. The page includes a small JavaScript snippet that **auto-reloads it every 5 seconds**, so the browser stays in sync with the LED matrix.

Because the number is generated on a timer (not per request), the LED matrix and the webpage always show the **same value**.

## Connecting and Viewing

1. Upload the sketch via USB.
2. Unplug USB (optional) and power the board another way.
3. On your phone or computer, connect to the Wi-Fi network:
   - **SSID:** `R4-Test`
   - **Password:** `12345678`
   - (A "no internet" warning is normal — keep the connection.)
4. Open a browser and go to:

   ```
   http://192.168.4.1
   ```

5. You should see the current random number. It refreshes automatically, and the number changes every 10 seconds — matching the LED matrix on the board.

### Why `192.168.4.1`?

In Access Point mode, the board **is** the router. The `WiFiS3` library assigns the board a fixed gateway address of `192.168.4.1` by default (same convention as ESP32-based APs). Your phone receives an address in the `192.168.4.x` range via DHCP from the board. `WiFi.localIP()` is printed to the Serial Monitor at startup to confirm this.

## The Sketch

```cpp
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
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<meta charset='UTF-8'>");
    client.println("<title>R4 Random Number</title>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>Random Number:</h1>");
    client.println("<p style='font-size:48px;'>" + String(currentNumber) + "</p>");
    client.println("<p>This page refreshes every 5 seconds; the number changes every 10 seconds.</p>");
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
```

## Key Concepts Demonstrated

| Concept | Explanation |
|---|---|
| Access Point mode | `WiFi.beginAP()` makes the board its own hotspot — no router or internet needed |
| Fixed IP | AP mode gives the board `192.168.4.1`, the gateway of its own network |
| Web server | `WiFiServer` + `client.println()` writes raw HTTP responses byte-by-byte |
| HTML/JS from the board | The webpage is generated as strings in C++ — any visualization must be emitted this way |
| Auto-refresh | JavaScript `setTimeout` + `location.reload()` reloads the page every 5 s |
| Shared state | `currentNumber` is generated on a timer, so LED matrix and webpage always match |
| LED matrix | `ArduinoGraphics` + `Arduino_LED_Matrix` libraries render text on the 12×8 matrix |

## Notes and Troubleshooting

- **Include order matters:** `ArduinoGraphics.h` must come **before** `Arduino_LED_Matrix.h`, otherwise the drawing functions (`beginDraw`, `stroke`, `textFont`, etc.) are not available.
- **Only one `setup()` and `loop()`:** duplicate definitions cause "redefinition" compile errors.
- **Browser won't load the page:** try typing `http://192.168.4.1` explicitly (some browsers default to HTTPS). Disable mobile data temporarily if the browser keeps using cellular.
- **USB:** only required for uploading the sketch. After that, any 5 V source (power bank, charger) will run the board.
- **No internet:** the hotspot provides only the local page; the board is not connected to the internet.

## Next Steps

- Replace `random(0, 1000)` with a real sensor reading (e.g., HC-SR04 ultrasonic distance).
- Alternatively, have the board return **plain text** (`Content-Type: text/plain`) and let a local p5.js/Processing sketch poll `http://192.168.4.1` for visualization or sound — offloading the drawing work to the computer.
