#pragma once
#include <Arduino.h>
#include "../Core/clsScreen.h"
#include "../Lib/clsButtons.h"
#include "clsWebDashboard.h"
#include "../Core/clsRandomPasswordGenerator.h"
#include <BleKeyboard.h>

extern BleKeyboard bleKeyboard;

/**
 * @class clsSettingScreen
 * 
 * 
 */
class clsSettingScreen : public clsScreen
{
private:
    clsButtons& _Buttons;
    clsWebDashboard _WebDashboard; // 
    bool _NeedRedraw = true;
    bool _IsPortalActive = false;  // 
    String _ApPassword = "";       // 

    /**
     * @brief Generates a secure, temporary 8-character password for the AP.
     * @return String The generated random password.
     */
    String _GenerateRandomPassword() {
        return clsRandomPasswordGenerator::GenerateWord(clsRandomPasswordGenerator::MixChars, 8).c_str();
    }

    /**
     * @brief Draws the Settings Portal screen layout on the OLED display.
     * Displays the static SSID and the dynamically generated AP Password.
     */
    void _DrawSettingsPortalScreen()
    {
        _Display.clearDisplay();
        _DrawScreenHeader("Settings Portal");

        _Display.setTextColor(SH110X_WHITE);
        _Display.setTextSize(1);
        
        // Render SSID
        _Display.setCursor(0, 24);
        _Display.print("SSID: Mr.PassKaper");
        
        // Render Password
        _Display.setCursor(0, 36);
        _Display.print("PASS: ");
        _Display.print(_ApPassword);

        // Render Instructions
        _Display.setCursor(0, 48);
        _Display.print("Hold Yellow: Send Pass");
        _Display.setCursor(0, 58);
        _Display.print("Hold Green: Exit");

        _Display.display();
    }

public:
    clsSettingScreen(Adafruit_SH1106G& display, clsButtons& buttons) 
        : clsScreen(display), _Buttons(buttons) {}

    /**
     * @brief Initializes the settings screen.
     * Generates a new random Wi-Fi password and starts the AP & Web Dashboard.
     */
    void init() {
        _Buttons.resetState();
        _NeedRedraw = true;
        _ApPassword = _GenerateRandomPassword();
        
        // Show loading text while starting Wi-Fi
        _Display.clearDisplay();
        _DrawScreenHeader("Settings Portal");
        _Display.setCursor(0, 35);
        _Display.print("Starting Wi-Fi...");
        _Display.display();

        // Start the AP and Captive Portal
        _IsPortalActive = _WebDashboard.Start("Mr.PassKaper", _ApPassword.c_str());
    }

    void Show() override {
        if (_IsPortalActive) {
            _DrawSettingsPortalScreen();
        } else {
            // Show error message if Wi-Fi fails to start
            _Display.clearDisplay();
            _DrawScreenHeader("Settings Portal");
            _Display.setCursor(0, 35);
            _Display.print("Wi-Fi Error!");
            _Display.display();
        }
    }

    /**
     * @brief Main loop update function for the settings screen.
     * Handles web server requests and button events.
     * @return bool True if exiting the settings screen, False otherwise.
     */
    bool Update() {
        // Handle incoming web requests if portal is active
        if (_IsPortalActive) {
            _WebDashboard.ProcessNextRequest();
        }

        enButtonEvent navEvent = _Buttons.getNavEvent();
        enButtonEvent selEvent = _Buttons.getSelEvent();

        // Yellow Button (Navigation Button) Long Press
        // Sends the AP Password via Bluetooth to the host device
        if (navEvent == BTN_LONG_PRESS) {
            if (bleKeyboard.isConnected()) {
                for (unsigned int i = 0; i < _ApPassword.length(); i++) {
                    bleKeyboard.print(_ApPassword[i]);
                    delay(15);
                }
            }
        }

        // Green Button (Select Button) Long Press
        // Exits the settings portal and shuts down the Wi-Fi AP
        if (selEvent == BTN_LONG_PRESS) {
            _WebDashboard.Stop(); // Disable web server & Wi-Fi
            _IsPortalActive = false;
            return true; // Return to main menu
        }

        if (_NeedRedraw) {
            Show();
            _NeedRedraw = false;
        }

        return false;
    }
};
