#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> 
#include <BleKeyboard.h>

// Include the main screen file, which in turn includes the rest of the screens and systems
#include "Core/clsMainScreen.h"

// -------------------------------------------------------------
// OLED Display Settings
// -------------------------------------------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Format the screen object with type SH1106G and use I2C (Wire) protocol
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------------------------------------------------------------
// BLE Keyboard Settings
// -------------------------------------------------------------
// Configure the keyboard with the name “PassKaper” and the manufacturer “PassKaper Dev”
BleKeyboard bleKeyboard("PassKaper", "PassKaper Dev", 100);

// -------------------------------------------------------------
// 
// -------------------------------------------------------------
// Create a home screen object and pass the screen to him so he can draw on it
clsMainScreen MainScreen(display);

void setup() {
    // Initiating serial communication for debugging purposes
    Serial.begin(115200);
    
    // Configuring the I2C protocol pins (SDA=21, SCL=22) of the ESP32
    Wire.begin(21, 22);

    // Trying to turn on the monitor at the default address 0x3C
    if (!display.begin(0x3C, true)) {
        Serial.println("SH1106 init failed");
        while (true); // Shut down the system if the screen fails to turn on
    }
    
    // Clean the screen of any random data on startup
    display.clearDisplay();
    
    // Start the Bluetooth keyboard to start broadcasting the signal to devices
    bleKeyboard.begin();
    
    // Added a 30ms delay between button presses
    // This is very important to ensure that characters are not lost during fast typing via Bluetooth
    bleKeyboard.setDelay(30); 
    
    // Configure the home screen (configure buttons and reset statuses)
    MainScreen.init(); 
    
    // Draw the main menu for the first time
    MainScreen.Update(); 
}

void loop() {
    // Continuous system updating (sensing buttons, switching screens, etc.)
    // All program logic is managed by this function in a hierarchical manner (OOP).
    MainScreen.Update(); 
}
