#pragma once

#include "state.h"

class app_t { 
    public:
        virtual void update(state_t state);
        virtual void draw();
        bool redraw_needed;
};