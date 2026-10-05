#pragma once
#include <Arduino.h>
#include "clsScreen.h" 
#include "../Lib/clsButtons.h"
#include "../AccountClasses/clsAccountsScreen.h"
#include "../SettingClasses/clsSettingScreen.h"

/**
 * @class clsMainScreen
 * 
 */
class clsMainScreen : public clsScreen
{
private:
    // 
    enum enMainMenueOptions { eAccounts = 1, eSetting = 2 };

    clsButtons _Buttons;               // 
    clsAccountsScreen _AccountsScreen; // 
    clsSettingScreen _SettingScreen;   // 

    int _CurrentMenuIndex = 0; // 
    int _MaxMenuIndex = 1;     // 
    bool _NeedRedraw = true;   // 

    bool _InAccountsScreen = false; // 
    bool _InSettingScreen = false;  // 

    /**
     * 
     * 
     */
    void _DrawMainMenu()
    {
        _Display.clearDisplay();
        _Display.setTextColor(SH110X_WHITE);
        _Display.setTextSize(1);
        _Display.setCursor(25, 0);
        _Display.println("Mr.PassKaper");

        _Display.setTextSize(1);
        
        // 
        _Display.setCursor(10, 20);
        if (_CurrentMenuIndex == 0) _Display.print("> ");
        else _Display.print("  ");
        _Display.println("1. Accounts");

        // 
        _Display.setCursor(10, 35);
        if (_CurrentMenuIndex == 1) _Display.print("> ");
        else _Display.print("  ");
        _Display.println("2. Setting");

        _Display.display(); // 
    }

    /**
     * 
     * 
     * 
     */
    void _PerfromMainMenueOption(enMainMenueOptions MainMenueOption)
    {
        switch (MainMenueOption)
        {
        case enMainMenueOptions::eAccounts:
            _AccountsScreen.init(); // 
            _InAccountsScreen = true;
            break;
        case enMainMenueOptions::eSetting:
            _SettingScreen.init(); // 
            _InSettingScreen = true;
            break;
        }
    }

public:
    /**
     * 
     * 
     * 
     */
    clsMainScreen(Adafruit_SH1106G& display) 
        : clsScreen(display), 
          _AccountsScreen(display, _Buttons),
          _SettingScreen(display, _Buttons)
    {
    }

    /**
     * 
     * 
     */
    void init()
    {
        _Buttons.init(); // 
        _CurrentMenuIndex = 0;
        _NeedRedraw = true;
        _InAccountsScreen = false;
        _InSettingScreen = false;
    }

    /**
     * 
     * 
     */
    void Show() override
    {
        if (_InAccountsScreen) {
            _AccountsScreen.Show();
        } else if (_InSettingScreen) {
            _SettingScreen.Show();
        } else {
            _DrawMainMenu();
        }
    }

    /**
     * 
     * 
     */
    void Update()
    {
        // 
        _Buttons.update(); 

        // 
        if (_InAccountsScreen) {
            if (_AccountsScreen.Update()) { 
                // 
                _Buttons.resetState();
                _InAccountsScreen = false; // 
                _NeedRedraw = true;        // 
            }
            return;
        }

        // 
        if (_InSettingScreen) {
            if (_SettingScreen.Update()) {
                // 
                _Buttons.resetState();
                _InSettingScreen = false; // 
                _NeedRedraw = true;       // 
            }
            return;
        }

        // 
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
            _PerfromMainMenueOption((enMainMenueOptions)(_CurrentMenuIndex + 1));
        }
        // 
        else if (selEvent == BTN_SHORT_PRESS) {
            _CurrentMenuIndex--;
            if (_CurrentMenuIndex < 0) _CurrentMenuIndex = _MaxMenuIndex; // 
            _NeedRedraw = true;
        }

        // 
        if (_NeedRedraw) {
            Show();
            _NeedRedraw = false;
        }
    }
};
