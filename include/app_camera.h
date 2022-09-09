#pragma once
#include "app.h"
#include "image.h"

#define READ_MLX_EVERY 250

class app_camera : public app_t {
    public:
        void draw();
        void update(state_t state);

        image_t image_buffer;

    private:        
        //allow us to redraw the MLX display only every other frame, to make the flickering less obvious? 
        bool odd_frame;

        //enable 'freeze frame' as sub for pictures
        bool freeze_frame;

        
        //readouts for current values on screen
        float temp_max;
        float temp_mid;
        float temp_min;
        
        size_t last_read_mlx = 0;
};