#pragma once

#include <Arduino.h>
#include <TouchScreen.h>

#define YP A4  // must be an analog pin, use "An" notation!
#define XM A7  // must be an analog pin, use "An" notation!
#define YM A6   // can be a digital pin
#define XP A5   // can be a digital pin
#define X_MIN  750
#define X_MAX  325
#define Y_MIN  840
#define Y_MAX  240

class touch_t {
    public:
        TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
        uint8_t previous_pressure = 0;

        bool touch;
        bool new_touch;
        TSPoint current_touch;
    
        void update();

    private:
        
};

inline touch_t touch;