#pragma once
#include <Arduino.h>
#include "../Core/clsScreen.h"
#include "../Lib/clsButtons.h"
#include <BleKeyboard.h>

// 
extern BleKeyboard bleKeyboard;

/**
 * @class clsSendAs
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
class clsSendAs : public clsScreen
{
private:
    clsButtons& _Buttons;
    String _AccountUsername; // 
    String _AccountPassword;

    // 
    static const int _OptionsCount = 3;
    const char* _Options[_OptionsCount] = {
        "Username",
        "Password",
        "User&Pass"
    };

    int _CurrentMenuIndex = 0;
    const int _MaxMenuIndex = _OptionsCount - 1; // = 2
    bool _NeedRedraw = true;

    /**
     * 
     * 
     */
    void _DrawSendAsMenu()
    {
        _Display.clearDisplay();
        _DrawScreenHeader("Send As"); // 

        for (int i = 0; i < _OptionsCount; i++) {
            int y = 20 + (i * 12); // 
            _Display.setCursor(0, y);
            
            // 
            if (i == _CurrentMenuIndex) _Display.print("> ");
            else _Display.print("  ");
            
            _Display.print(i + 1);
            _Display.print(". ");
            _Display.println(_Options[i]);
        }

        _Display.display(); // 
    }

    /**
     * 
     * 
     * 
     * 
     * 
     * 
     * 
     * 
     */
    void _TypeString(String text) {
        for (unsigned int i = 0; i < text.length(); i++) {
            bleKeyboard.print(text[i]); // 
            delay(15); // 
        }
    }

    /**
     * 
     * 
     * 
     * 
     * 
     * 
     * 
     * 
     * 
     * 
     */
    void _PerformAction(int index)
    {
        _Display.clearDisplay();
        _DrawScreenHeader("Sending..."); // 
        _Display.setCursor(0, 30);
        
        if (bleKeyboard.isConnected()) {
            switch(index) {
                case 0: // 
                    _Display.print("Sending Username"); 
                    _Display.display();
                    _TypeString(_AccountUsername);
                    break;
                    
                case 1: // 
                    _Display.print("Sending Password"); 
                    _Display.display();
                    _TypeString(_AccountPassword);
                    break;
                    
                case 2: // 
                    _Display.print("Sending Both"); 
                    _Display.display();
                    _TypeString(_AccountUsername);
                    bleKeyboard.write(KEY_TAB);   // 
                    delay(100);                    // 
                    _TypeString(_AccountPassword);
                    bleKeyboard.write(KEY_RETURN); // 
                    break;
            }
        } else {
            // 
            _Display.print("BLE Disconnected!");
            _Display.display();
        }

        delay(1500); // 
        _NeedRedraw = true; // 
    }

public:
    clsSendAs(Adafruit_SH1106G& display, clsButtons& buttons) 
        : clsScreen(display), _Buttons(buttons) {}

    /**
     * 
     * 
     * 
     * 
     */
    void init(String username, String password) {
        _AccountUsername = username;
        _AccountPassword = password;
        _Buttons.resetState();   // 
        _CurrentMenuIndex = 0;   // 
        _NeedRedraw = true;
    }

    void Show() override {
        _DrawSendAsMenu();
    }

    /**
     * 
     * 
     */
    bool Update() {
        enButtonEvent navEvent = _Buttons.getNavEvent();
        enButtonEvent selEvent = _Buttons.getSelEvent();

        // 
        if (navEvent == BTN_SHORT_PRESS) {
            _CurrentMenuIndex++;
            if (_CurrentMenuIndex > _MaxMenuIndex) _CurrentMenuIndex = 0; // 
            _NeedRedraw = true;
        }
        // 
        else if (navEvent == BTN_LONG_PRESS) {
            _PerformAction(_CurrentMenuIndex);
        }

        // 
        if (selEvent == BTN_SHORT_PRESS) {
            _CurrentMenuIndex--;
            if (_CurrentMenuIndex < 0) _CurrentMenuIndex = _MaxMenuIndex; // 
            _NeedRedraw = true;
        }
        // 
        else if (selEvent == BTN_LONG_PRESS) {
            return true; // 
        }

        if (_NeedRedraw) {
            Show();
            _NeedRedraw = false;
        }

        return false; // 
    }
};
