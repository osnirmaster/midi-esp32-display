# ESP32‑C6 MIDI Display

This project demonstrates how to build a versatile MIDI display/controller using the Waveshare **ESP32‑C6 LCD 1.47" development board**.  The goal is to create a companion device for guitar pedals like the **Tonex One** and **Kemper Player**, showing the currently selected preset/rig on the built‑in display, allowing the user to switch presets via buttons, and offering both Bluetooth MIDI and wired MIDI connectivity.  A simple Wi‑Fi web console provides configuration options such as the pedal type and the background colour of the display.

## Features

- **Built‑in ST7789 display** (172 × 320) driven by the [`TFT_eSPI`](https://github.com/Bodmer/TFT_eSPI) library.  The sketch draws the pedal name, preset number and preset name, and a simple bar‑style VU meter based on an analog input.  To use the display with this board you must copy the `Setup701_C6_WS_ST7789_172x320.h` file into the `TFT_eSPI/User_Setups` folder and enable it in `User_Setup_Select.h`, as documented in the Waveshare wiki and companion example [oai_citation:0‡waveshare.com](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47#:~:text=).
- **BLE MIDI server** implemented using the [`ESP32-BLE-MIDI`](https://github.com/TedBradley/ESP32-BLE-MIDI) library.  The code creates a BLE MIDI service named **ESP32C6-MIDI**, accepts connections from your pedal or DAW, and updates the display on incoming Program Change or Control Change messages.
- **Wi‑Fi configuration portal**: when the device boots, it starts a Wi‑Fi access point named `ESP32C6-MIDI-CONFIG`.  Navigating to `http://192.168.4.1` opens a basic web page where you can select the pedal type (Tonex One or Kemper Player) and choose a background colour.  Settings are saved to the ESP32’s non‑volatile storage (NVS) using the `Preferences` library [oai_citation:1‡docs.espressif.com](https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/preferences.html#:~:text=Introduction%EF%83%81).
- **Storage for presets**: the code keeps an array of preset names and stores them in NVS.  Program Change messages increment the current preset and update the display accordingly.
- **Expandable I/O**: two pushbuttons (wired to GPIO 18 and 19 in the Wokwi diagram) act as “Next” and “Prev” footswitches, and a potentiometer on GPIO 3 demonstrates reading analog values for a VU meter.  You can add more inputs such as rotary encoders or MIDI DIN jacks by connecting them to free GPIO pins.

## Files

| File | Description |
| --- | --- |
| `sketch.ino` | Arduino code implementing the MIDI display, BLE server, Wi‑Fi portal and settings storage.  It uses the built‑in display to show the pedal name, preset number, preset name and a simple VU meter. |
| `diagram.json` | Wokwi diagram for simulating the ESP32‑C6 board.  It instantiates a `board-esp32-c6-devkitc-1`, two `wokwi-pushbutton`s for Next/Prev and a `wokwi-potentiometer` for the VU meter.  The buttons are connected to GPIO 18 and 19 and the potentiometer to GPIO 3 with 3.3 V and GND. |

## How to run in Wokwi

1. Visit [wokwi.com](https://wokwi.com/) and create a new project.  Copy the contents of `sketch.ino` and `diagram.json` into the respective files.  Wokwi supports the `board-esp32-c6-devkitc-1` microcontroller, which will execute your Arduino sketch [oai_citation:2‡docs.wokwi.com](https://docs.wokwi.com/diagram-format#:~:text=microcontroller%20part%20that%20will%20execute,following%20microcontrollers%20are%20currently%20supported).
2. Click **Start** to build and run the simulation.  The display will show the pedal name and preset information.  You can click the virtual pushbuttons to simulate footswitch presses and turn the potentiometer to see the VU meter update.
3. To configure the device, connect to the simulated Wi‑Fi access point `ESP32C6-MIDI-CONFIG` (Wokwi provides a network simulation) and open `http://192.168.4.1` in your browser.  Choose the pedal type and background colour, then click **Save**.  The display updates immediately.

## Real hardware wiring

The Waveshare ESP32‑C6 board has an on‑board 1.47″ display and micro‑SD slot.  To build a practical MIDI controller around it you will need a few extra components:

- **Momentary footswitches** or **buttons** for navigating presets.  Connect one side of each switch to ground and the other side to a GPIO pin with an input pull‑up.  In the example we use GPIO 18 (Next) and GPIO 19 (Prev).  You can choose different pins as long as they are free.
- **Rotary encoder** (optional) for selecting presets or controlling parameters.  Connect the encoder’s A/B pins to two GPIOs and the pushbutton to a third GPIO.
- **MIDI DIN connectors** for wired MIDI In/Out.  Each requires an opto‑isolation circuit (e.g. 6N137 or PC900) for the IN port and a current‑limited output driver for the OUT port.  The schematic recommended by the MIDI spec uses a 220 Ω resistor and a standard 5‑pin DIN jack.
- **Envelope follower / VU meter input** (optional) to display gain or EQ values.  A simple diode‑capacitance rectifier feeding an analog pin can provide an approximate level meter.
- **Power supply**: the ESP32‑C6 board can be powered via USB‑C or a regulated 5 V input.  Make sure to use a supply capable of delivering at least 500 mA for the display and BLE radio.

Refer to the Waveshare wiki for complete pin descriptions.  The display’s SPI interface uses GPIO 6 (MOSI), 7 (SCLK), 14 (CS), 15 (DC), 21 (RST) and 22 (backlight) [oai_citation:3‡waveshare.com](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47#:~:text=).

## Limitations and next steps

This example provides a starting point rather than a finished product.  You will need to:

1. **Implement a full MIDI parser** if you want to display parameter values such as gain or EQ.  Consider using a dedicated MIDI library for parsing messages and mapping them to on‑screen widgets.
2. **Extend the web interface** to upload and edit preset names and to select from more pedal types.  A JSON API would make integration with external editors easier.
3. **Add hardware components** like DIN MIDI connectors, rotary encoders, footswitches with LEDs, or a micro‑SD card for storing large numbers of presets.
4. **Create a custom enclosure** with labelled buttons and a diffuser for the display.  A laser‑cut acrylic case would protect the electronics while keeping the unit compact.

## Repository creation

Due to limitations of the available GitHub API connector, this assistant cannot programmatically create a new repository in your GitHub account.  To store this code on GitHub you can:

1. Manually create a new repository in your GitHub account (e.g. `esp32c6-midi-display`).
2. Clone the repository locally or use the GitHub web UI to add files.
3. Copy the contents of `sketch.ino`, `diagram.json` and `README.md` into the repository and commit them.

Once pushed, you can continue developing the firmware and share the project with others for feedback and collaboration.