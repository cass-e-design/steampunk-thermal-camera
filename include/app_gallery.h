#pragma once
#include "app.h"
#include "image.h"

class app_gallery : public app_t {
    public:
        void draw();
        void update(state_t state);

        image_t image_buffer;
        //todo: next / prev buffer?

    private:        
        //enable 'freeze frame' as sub for pictures
        bool freeze_frame;

        //readouts for current values on screen
        float temp_max;
        float temp_mid;
        float temp_min;
        
        size_t last_read_mlx = 0;
};