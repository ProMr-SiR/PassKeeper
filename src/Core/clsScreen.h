#pragma once
#include <Adafruit_SH110X.h>

class clsScreen {
protected:
    Adafruit_SH1106G& _Display;

    void _DrawScreenHeader(const char* title, const char* subTitle = nullptr) {
        _Display.setTextColor(SH110X_WHITE);
        _Display.setTextSize(1);
        _Display.setCursor(0, 0);
        _Display.print(title);

        int lineY = 12;
        if (subTitle != nullptr) {
            _Display.setCursor(0, 12);
            _Display.print(subTitle);
            lineY = 22;
        }
        _Display.drawLine(0, lineY, 127, lineY, SH110X_WHITE);
    }

public:
    explicit clsScreen(Adafruit_SH1106G& display) : _Display(display) {}
    virtual ~clsScreen() = default;
    virtual void Show() = 0;
};
