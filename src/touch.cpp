#include "touch.h"

void touch_t::update() {
    previous_pressure = current_touch.z;
    if (new_touch) new_touch = false;

    current_touch = ts.getPoint();
    touch = current_touch.z >= 1;
    
    if (touch && previous_pressure < 1) new_touch = true;
}