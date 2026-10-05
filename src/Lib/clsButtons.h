#pragma once
#include <Arduino.h>

// =============================================================
// 
// 
// 
// =============================================================
#define BTN_NAV 4
#define BTN_SEL 5

// =============================================================
// 
// 
// 
// 
// =============================================================
enum enButtonEvent {
    BTN_NO_EVENT = 0,
    BTN_SHORT_PRESS,
    BTN_LONG_PRESS
};

/**
 * @class clsButtons
 * 
 * 
 * 
 * 
 * 
 * 
 */
class clsButtons
{
private:
    // 
    const unsigned long DEBOUNCE_DELAY = 40;   // 
    const unsigned long LONG_PRESS_MS  = 600;  // 

    // ==========================================================
    // 
    // 
    // ==========================================================
    bool _NavPressed         = false;  // 
    bool _NavLongTriggered   = false;  // 
    unsigned long _NavPressStart = 0;  // 
    int _LastNavReading      = LOW;   // 
    unsigned long _LastNavDebounce = 0;// 

    // ==========================================================
    // 
    // ==========================================================
    bool _SelPressed         = false;
    bool _SelLongTriggered   = false;
    unsigned long _SelPressStart = 0;
    int _LastSelReading      = HIGH;   // 
    unsigned long _LastSelDebounce = 0;

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
     * 
     * 
     * 
     * 
     * 
     * 
     */
    enButtonEvent _ProcessButton(int pin, bool activeHigh, 
                                 bool& isPressed, bool& longTriggered,
                                 unsigned long& pressStart, 
                                 int& lastReading, unsigned long& lastDebounce)
    {
        int reading = digitalRead(pin);   // 
        unsigned long now = millis();     // 

        // 
        if (reading != lastReading) {
            lastDebounce = now;
            lastReading = reading;
        }

        // 
        if ((now - lastDebounce) >= DEBOUNCE_DELAY) {

            // 
            bool pressed = (activeHigh ? (reading == HIGH) : (reading == LOW));

            if (pressed) {
                if (!isPressed) {
                    // 
                    isPressed = true;
                    pressStart = now;
                    longTriggered = false;
                } else if (!longTriggered && (now - pressStart >= LONG_PRESS_MS)) {
                    // 
                    longTriggered = true;
                    return BTN_LONG_PRESS;
                }
            } else {
                if (isPressed) {
                    if (!longTriggered) {
                        // 
                        isPressed = false;
                        longTriggered = false;
                        return BTN_SHORT_PRESS;
                    }
                    // 
                    isPressed = false;
                    longTriggered = false;
                }
            }
        }
        return BTN_NO_EVENT; // 
    }

public:
    /**
     * 
     * 
     */
    void init()
    {
        // 
        pinMode(BTN_NAV, INPUT_PULLDOWN);
        // 
        pinMode(BTN_SEL, INPUT_PULLUP);
        resetState();
    }

    /**
     * 
     * 
     * 
     * 
     */
    void resetState()
    {
        unsigned long now = millis();

        // 
        bool navPhysical = (digitalRead(BTN_NAV) == HIGH);
        _NavPressed         = navPhysical;
        _NavLongTriggered   = navPhysical;
        _NavPressStart      = now;
        _LastNavReading     = digitalRead(BTN_NAV);
        _LastNavDebounce    = now;

        // 
        bool selPhysical = (digitalRead(BTN_SEL) == LOW);
        _SelPressed         = selPhysical;
        _SelLongTriggered   = selPhysical;
        _SelPressStart      = now;
        _LastSelReading     = digitalRead(BTN_SEL);
        _LastSelDebounce    = now;
    }

    /**
     * 
     * @return BTN_NO_EVENT / BTN_SHORT_PRESS / BTN_LONG_PRESS
     */
    enButtonEvent getNavEvent() {
        return _ProcessButton(BTN_NAV, true,
                              _NavPressed, _NavLongTriggered,
                              _NavPressStart, _LastNavReading, _LastNavDebounce);
    }

    /**
     * 
     * @return BTN_NO_EVENT / BTN_SHORT_PRESS / BTN_LONG_PRESS
     */
    enButtonEvent getSelEvent() {
        return _ProcessButton(BTN_SEL, false,
                              _SelPressed, _SelLongTriggered,
                              _SelPressStart, _LastSelReading, _LastSelDebounce);
    }
};
