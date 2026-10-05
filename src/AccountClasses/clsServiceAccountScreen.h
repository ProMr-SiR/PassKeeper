#pragma once
#include <Arduino.h>
#include "../Core/clsScreen.h"
#include "../Core/clsPasswordManager.h"
#include "../Lib/clsButtons.h"
#include "clsSendAs.h"
#include <vector>
#include <string>

using namespace std;

/**
 * @class clsServiceAccountScreen
 * 
 * 
 */
class clsServiceAccountScreen : public clsScreen
{
private:
    clsButtons& _Buttons;
    clsSendAs _SendAsScreen; // 

    string _ServiceName; // 
    vector<clsPasswordManager> _Accounts; // 

    int _CurrentMenuIndex = 0;
    int _MaxMenuIndex = 0;
    bool _NeedRedraw = true;
    bool _InSendAsScreen = false; // 

    /**
     * 
     */
    void _LoadAccounts() {
        _Accounts = clsPasswordManager::GetEntriesByService(_ServiceName);
        _MaxMenuIndex = (int)_Accounts.size() - 1;
        if (_MaxMenuIndex < 0) _MaxMenuIndex = 0;
        if (_CurrentMenuIndex > _MaxMenuIndex && _MaxMenuIndex >= 0) 
            _CurrentMenuIndex = _MaxMenuIndex;
    }

    /**
     * 
     * 
     */
    void _DrawAccountsList()
    {
        _Display.clearDisplay();
        
        // 
        String header = _ServiceName.c_str();
        _DrawScreenHeader(header.c_str());

        if (_Accounts.empty()) {
            _Display.setCursor(5, 30);
            _Display.print("No accounts.");
            _Display.display();
            return;
        }

        const int maxVisible = 3;
        int startIndex = 0;
        int count = (int)_Accounts.size();
        
        // 
        if (_CurrentMenuIndex >= maxVisible)
            startIndex = _CurrentMenuIndex - maxVisible + 1;

        // 
        for (int i = startIndex; i < count && (i - startIndex) < maxVisible; i++) {
            int y = 20 + ((i - startIndex) * 12);
            _Display.setCursor(0, y);
            
            if (i == _CurrentMenuIndex) _Display.print("> ");
            else _Display.print("  ");
            
            _Display.print(i + 1);
            _Display.print(". ");

            String user = _Accounts[i].Username().c_str();
            
            // 
            if (user.length() > 14) {
                user = user.substring(0, 12) + "..";
            }
            
            _Display.println(user);
        }

        _Display.display();
    }

    /**
     * 
     * 
     */
    void _EnterSendAs(int index)
    {
        if (index < 0 || index >= (int)_Accounts.size()) return;
        _Buttons.resetState();
        
        // 
        String u = _Accounts[index].Username().c_str();
        String p = _Accounts[index].Password().c_str();
        _SendAsScreen.init(u, p);
        
        _InSendAsScreen = true;
    }

public:
    clsServiceAccountScreen(Adafruit_SH1106G& display, clsButtons& buttons) 
        : clsScreen(display), _Buttons(buttons),
          _SendAsScreen(display, buttons) {}

    /**
     * 
     */
    void init(string serviceName)
    {
        _ServiceName = serviceName;
        _Buttons.resetState();
        _CurrentMenuIndex = 0;
        _NeedRedraw = true;
        _InSendAsScreen = false;
        _LoadAccounts();
    }

    void Show() override {
        if (_InSendAsScreen) {
            _SendAsScreen.Show();
        } else {
            _DrawAccountsList();
        }
    }

    /**
     * 
     * 
     */
    bool Update()
    {
        if (_InSendAsScreen) {
            if (_SendAsScreen.Update()) { // 
                _Buttons.resetState();
                _InSendAsScreen = false;
                _NeedRedraw = true;
            }
            return false;
        }

        enButtonEvent navEvent = _Buttons.getNavEvent();
        enButtonEvent selEvent = _Buttons.getSelEvent();

        if (_Accounts.empty()) {
            if (selEvent == BTN_LONG_PRESS) {
                return true; // 
            }
            if (_NeedRedraw) { Show(); _NeedRedraw = false; }
            return false;
        }

        // 
        if (navEvent == BTN_SHORT_PRESS) {
            _CurrentMenuIndex++;
            if (_CurrentMenuIndex > _MaxMenuIndex) _CurrentMenuIndex = 0;
            _NeedRedraw = true;
        }
        // 
        else if (navEvent == BTN_LONG_PRESS) {
            _EnterSendAs(_CurrentMenuIndex);
        }

        // 
        if (selEvent == BTN_SHORT_PRESS) {
            _CurrentMenuIndex--;
            if (_CurrentMenuIndex < 0) _CurrentMenuIndex = _MaxMenuIndex;
            _NeedRedraw = true;
        }
        // 
        else if (selEvent == BTN_LONG_PRESS) {
            return true;
        }

        if (_NeedRedraw) {
            Show();
            _NeedRedraw = false;
        }

        return false;
    }
};
