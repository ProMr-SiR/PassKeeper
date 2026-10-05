#pragma once
#include <Arduino.h>
#include "../Core/clsScreen.h"
#include "../Core/clsPasswordManager.h"
#include "../Lib/clsButtons.h"
#include "clsServiceAccountScreen.h"
#include <vector>
#include <string>

using namespace std;

/**
 * @class clsAccountsScreen
 * @brief Displays a list of services (e.g. Google, Github) that contain saved accounts.
 * It allows you to navigate between the names of services and choose one of them to go to the accounts screen for that service.
 */
class clsAccountsScreen : public clsScreen
{
private:
    clsButtons& _Buttons; // A reference to the Buttons object to receive input

    // One general sub-screen that is reused to display accounts for any service (passing the service name to it)
    clsServiceAccountScreen _ServiceScreen;

    // List of unique service names (so as not to repeat the same service twice)
    vector<string> _ServiceNames;

    int _CurrentMenuIndex = 0; // The current pointer to the list
    int _MaxMenuIndex = 0;     // Maximum index
    bool _NeedRedraw = true;   // Flag to reduce screen refresh
    bool _InSubScreen = false; // Are we currently inside the accounts screen for a particular service?

    /**
     * @brief Loads service names from memory and defines list boundaries.
     * 
     */
    void _LoadServiceNames() {
        _ServiceNames = clsPasswordManager::GetUniqueServiceNames();
        _MaxMenuIndex = (int)_ServiceNames.size() - 1;
        if (_MaxMenuIndex < 0) _MaxMenuIndex = 0; // Protection from negative values
        
        // If the indicator is greater than the number (for example, if a service is deleted), it is corrected
        if (_CurrentMenuIndex > _MaxMenuIndex && _MaxMenuIndex >= 0) 
            _CurrentMenuIndex = _MaxMenuIndex;
    }

    /**
     * @brief Draws a list of available services on the screen.
     * It supports the “smart scrolling” system to display a maximum of 3 services on one screen.
     */
    void _DrawAccountsList()
    {
        _Display.clearDisplay();
        _DrawScreenHeader("Accounts"); // Use the parent class function to draw the header

        // If there is no service reserved yet
        if (_ServiceNames.empty()) {
            _Display.setCursor(5, 25);
            _Display.print("No accounts yet.");
            _Display.setCursor(5, 37);
            _Display.print("Go to Settings to");
            _Display.setCursor(5, 49);
            _Display.print("add accounts via WiFi");
            _Display.display();
            return;
        }

        const int maxVisible = 3; // The maximum number shown on the screen
        int startIndex = 0;       // Scrolling start pointer
        int count = (int)_ServiceNames.size();
        
        // Smart scrolling logic: If the cursor descends below the visible items, the list is pulled down
        if (_CurrentMenuIndex >= maxVisible)
            startIndex = _CurrentMenuIndex - maxVisible + 1;

        // Draw only the visible elements
        for (int i = startIndex; i < count && (i - startIndex) < maxVisible; i++) {
            int y = 20 + ((i - startIndex) * 12);
            _Display.setCursor(0, y);
            
            if (i == _CurrentMenuIndex) _Display.print("> ");
            else _Display.print("  ");
            
            _Display.print(i + 1);
            _Display.print(". ");

            // Display the name of the service with the number of accounts saved within it
            String name = _ServiceNames[i].c_str();
            int accountCount = clsPasswordManager::GetEntriesByService(_ServiceNames[i]).size();
            _Display.print(name);
            _Display.print(" (");
            _Display.print(accountCount); // Number of accounts
            _Display.println(")");
        }

        _Display.display();
    }

    /**
     * @brief Moves the user to the sub-screen (the specific service accounts screen).
     * @param index The service number in the list.
     */
    void _EnterServiceScreen(int index)
    {
        if (index < 0 || index >= (int)_ServiceNames.size()) return; // protection
        _Buttons.resetState(); // Zero the buttons before moving on
        _ServiceScreen.init(_ServiceNames[index]); // Scroll the service name to the sub-screen
        _InSubScreen = true;
    }

public:
    /**
     * @brief Constructor: Passes the screen and buttons to the parents and to the child screen.
     */
    clsAccountsScreen(Adafruit_SH1106G& display, clsButtons& buttons) 
        : clsScreen(display), _Buttons(buttons),
          _ServiceScreen(display, buttons) {}

    /**
     * @brief Initialize the Services screen, called when entering this screen for the first time.
     */
    void init()
    {
        _Buttons.resetState();
        _CurrentMenuIndex = 0;
        _NeedRedraw = true;
        _InSubScreen = false;
        _LoadServiceNames(); // Fetch the latest data from flash
    }

    /**
     * @brief Display function.
     * It directs the display command to the sub-screen if we are inside it, otherwise it displays the services menu.
     */
    void Show() override {
        if (_InSubScreen) {
            _ServiceScreen.Show();
        } else {
            _DrawAccountsList();
        }
    }

    /**
     * @brief Update and read events function for this screen.
     * @return true if the user presses return to be taken to the main menu.
     */
    bool Update()
    {
        // If we are inside a subservice screen, we pass control to it
        if (_InSubScreen) {
            if (_ServiceScreen.Update()) {
                // If it returns true it means that the user pressed return to the current screen
                _Buttons.resetState();
                _InSubScreen = false;
                _LoadServiceNames(); // Reload the list due to the possibility that the user deleted the account from the web and came back
                _NeedRedraw = true;
            }
            return false;
        }

        enButtonEvent navEvent = _Buttons.getNavEvent();
        enButtonEvent selEvent = _Buttons.getSelEvent();

        // If there are no services, we will only accept returns (green is long)
        if (_ServiceNames.empty()) {
            if (selEvent == BTN_LONG_PRESS) {
                return true; // Request a return
            }
            if (_NeedRedraw) { Show(); _NeedRedraw = false; }
            return false;
        }

        // Short press yellow: scroll down
        if (navEvent == BTN_SHORT_PRESS) {
            _CurrentMenuIndex++;
            if (_CurrentMenuIndex > _MaxMenuIndex) _CurrentMenuIndex = 0;
            _NeedRedraw = true;
        }
        // Yellow long press: Enter the selected service accounts page
        else if (navEvent == BTN_LONG_PRESS) {
            _EnterServiceScreen(_CurrentMenuIndex);
        }

        // Green short press: scroll up
        if (selEvent == BTN_SHORT_PRESS) {
            _CurrentMenuIndex--;
            if (_CurrentMenuIndex < 0) _CurrentMenuIndex = _MaxMenuIndex;
            _NeedRedraw = true;
        }
        // Long press Green: Request to return to the main menu (return true to the home screen)
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
