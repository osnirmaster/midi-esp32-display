// ESP32-C6 MIDI Display project
//
// This Arduino sketch demonstrates a simple framework for building a MIDI
// controller/display based on the Waveshare ESP32‑C6 development board with
// integrated 1.47‑inch ST7789 display.  The code is written for the
// Arduino framework and uses the TFT_eSPI graphics library to drive the
// on‑board display, the Preferences library for saving settings to
// non‑volatile memory, the BLE MIDI library for MIDI over Bluetooth
// connectivity, and a simple HTTP web server for configuration when the
// device is connected via Wi‑Fi.  The goal of this sketch is to show how
// you might structure your own firmware to display preset names,
// assignable colours and basic parameter information from MIDI messages.
//
// NOTE: To use the TFT_eSPI library with the ESP32‑C6 LCD board you
// must copy the provided setup file (Setup701_C6_WS_ST7789_172x320.h)
// into the TFT_eSPI library's User_Setups folder and enable it in
// User_Setup_Select.h as described in the Waveshare tutorial.  See
// https://github.com/AndroidCrypto/ESP32_C6_Waveshare_ST7789_Starter for
// more details on configuring the display library.

#include <Arduino.h>
#include <TFT_eSPI.h>        // Graphics library (requires custom setup file)
#include <Preferences.h>      // Non‑volatile storage for user settings
#include <WiFi.h>             // Wi‑Fi connectivity
#include <WebServer.h>        // Simple HTTP server
#include <BLEDevice.h>        // Core BLE functions
#include <BLEMidi.h>          // BLE MIDI server (https://github.com/TedBradley/ESP32-BLE-MIDI)

// Create global objects
TFT_eSPI tft = TFT_eSPI();       // Screen driver (pins defined in setup file)
Preferences prefs;               // Non‑volatile preferences store
WebServer server(80);            // HTTP server on port 80

// BLE MIDI server instance
BLEMidiServer* midiServer;

// Application variables
String pedalType;                // Name of the pedal (Tonex One, Kemper Player, etc.)
uint16_t backgroundColor;        // Background colour (16‑bit 565)
uint8_t currentPreset = 0;       // Currently selected preset index
String presetNames[32];          // Array of preset names (max 32 entries)
uint8_t presetCount = 0;         // Number of stored presets

// Forward declarations
void loadSettings();
void saveSettings();
void handleRoot();
void handleSave();
void updateDisplay();
void handleMidiMessage(uint8_t status, uint8_t data1, uint8_t data2);

// Setup function runs once on boot
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting MIDI Display...");

  // Initialise preferences storage
  prefs.begin("settings", false);
  loadSettings();

  // Initialise TFT display (172 x 320 pixels).  Rotation=1 draws in
  // landscape orientation.  Without the correct user setup file the
  // following call may hang or produce no output.
  tft.init(172, 320);
  tft.setRotation(1);
  tft.fillScreen(backgroundColor);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, backgroundColor);

  // Display boot splash
  tft.setCursor(10, 10);
  tft.println(F("ESP32‑C6 MIDI Display"));
  tft.println();
  tft.print(F("Pedal: "));
  tft.println(pedalType);

  // Initialise BLE MIDI server
  BLEDevice::init("ESP32C6 MIDI Display");
  midiServer = BLEMidiServer::begin("ESP32C6-MIDI");
  // Register callback for incoming MIDI messages
  midiServer->setOnReceiveCallback(handleMidiMessage);

  // Set Wi‑Fi to access point mode for configuration
  WiFi.mode(WIFI_AP);
  const char *apName = "ESP32C6-MIDI-CONFIG";
  const char *apPass = "12345678";
  WiFi.softAP(apName, apPass);
  Serial.print("Started access point: ");
  Serial.println(apName);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Configure HTTP server routes
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
  Serial.println("HTTP server started");

  // Draw initial preset information
  updateDisplay();
}

// Main loop: handle web server requests and update display periodically
void loop() {
  // Handle any pending HTTP requests
  server.handleClient();

  // Additional application logic can be placed here.  For example,
  // scanning for button presses on GPIO pins to change presets or
  // reading analog input for a VU meter can be implemented.  This
  // skeleton simply waits for incoming MIDI messages and web
  // configuration.

  delay(10);
}

// Load settings from NVS (Preferences)
void loadSettings() {
  pedalType = prefs.getString("pedalType", "Tonex One");
  backgroundColor = prefs.getUShort("bgColor", TFT_BLACK);
  presetCount = prefs.getUChar("presetCount", 0);
  for (uint8_t i = 0; i < presetCount && i < 32; i++) {
    char key[12];
    sprintf(key, "pr%02u", i);
    presetNames[i] = prefs.getString(key, String("Preset ") + i);
  }
}

// Save settings to NVS
void saveSettings() {
  prefs.putString("pedalType", pedalType);
  prefs.putUShort("bgColor", backgroundColor);
  prefs.putUChar("presetCount", presetCount);
  for (uint8_t i = 0; i < presetCount && i < 32; i++) {
    char key[12];
    sprintf(key, "pr%02u", i);
    prefs.putString(key, presetNames[i]);
  }
}

// Generate simple HTML configuration page
void handleRoot() {
  String html;
  html += F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
  html += F("<title>MIDI Display Config</title></head><body style='font-family:Arial;'>\n");
  html += F("<h2>ESP32‑C6 MIDI Display Configuration</h2>\n");
  html += F("<form action='/save' method='get'>\n");
  html += F("Pedal type:<br>\n");
  html += F("<select name='pedal'>\n");
  html += F("<option value='Tonex One' ");
  if (pedalType == "Tonex One") html += F("selected");
  html += F(">Tonex One</option>\n");
  html += F("<option value='Kemper Player' ");
  if (pedalType == "Kemper Player") html += F("selected");
  html += F(">Kemper Player</option>\n");
  html += F("</select><br><br>\n");
  // Colour picker: convert 16‑bit colour to 24‑bit hex string
  uint8_t r = ((backgroundColor >> 11) & 0x1F) * 255 / 31;
  uint8_t g = ((backgroundColor >> 5) & 0x3F) * 255 / 63;
  uint8_t b = (backgroundColor & 0x1F) * 255 / 31;
  char colorHex[8];
  sprintf(colorHex, "%02X%02X%02X", r, g, b);
  html += F("Background colour:<br>\n");
  html += String("<input type='color' name='bg' value='#") + colorHex + String("'><br><br>\n");
  html += F("<input type='submit' value='Save'>\n");
  html += F("</form><hr>\n");
  html += F("<p>Current preset: ");
  html += String(currentPreset);
  html += F("</p>\n");
  html += F("</body></html>");
  server.send(200, "text/html", html);
}

// Handle saving configuration via query parameters
void handleSave() {
  if (server.hasArg("pedal")) {
    pedalType = server.arg("pedal");
  }
  if (server.hasArg("bg")) {
    String hex = server.arg("bg");
    // Remove '#' if present and convert to 16‑bit 565 value
    if (hex.startsWith("#")) hex.remove(0, 1);
    unsigned long color24 = strtoul(hex.c_str(), nullptr, 16);
    uint8_t r = (color24 >> 16) & 0xFF;
    uint8_t g = (color24 >> 8) & 0xFF;
    uint8_t b = color24 & 0xFF;
    backgroundColor = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }
  saveSettings();
  // Update display with new settings
  updateDisplay();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated");
}

// Update the TFT display with the current preset and settings
void updateDisplay() {
  tft.fillScreen(backgroundColor);
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println(pedalType);
  tft.setTextSize(1);
  tft.println();
  tft.print(F("Preset "));
  tft.println(currentPreset + 1);
  if (currentPreset < presetCount) {
    tft.print(F("Name: ")); tft.println(presetNames[currentPreset]);
  } else {
    tft.print(F("Name: ---"));
  }
  // Placeholder: draw a simple VU meter using analogue input (e.g. A0)
  int value = analogRead(3); // reads voltage on GPIO3 (choose your pin)
  int barLength = map(value, 0, 4095, 0, 160);
  tft.fillRect(10, 140, barLength, 10, TFT_GREEN);
  tft.drawRect(10, 140, 160, 10, TFT_WHITE);
}

// MIDI message handler
void handleMidiMessage(uint8_t status, uint8_t data1, uint8_t data2) {
  // This callback is invoked whenever a BLE MIDI message is received.
  // You can parse incoming Program Change or Control Change messages
  // from your pedal here and update the display accordingly.
  uint8_t command = status & 0xF0;
  if (command == 0xC0) { // Program Change: update current preset
    currentPreset = data1;
    updateDisplay();
  } else if (command == 0xB0) { // Control Change: update parameters
    // Example: interpret CC #20 as gain and #21 as EQ parameter
    if (data1 == 20) {
      // Map data2 (0–127) to bar colour or length, etc.
    } else if (data1 == 21) {
      // Handle EQ parameter
    }
  }
  // Send through to connected devices if required
}