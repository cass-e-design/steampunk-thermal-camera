#include "app.h"

void app_t::update(state_t state) { 
    if (redraw_needed) draw();
}

void app_t::draw() {
    redraw_needed = false;
}