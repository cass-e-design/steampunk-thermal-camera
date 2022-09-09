#pragma once

#include "Adafruit_MLX90640.h"
#include "image.h"

#define THRMLSENS_WIDTH 32
#define THRMLSENS_HEIGHT 24

class thermal_sensor_t {
    public:
        Adafruit_MLX90640 mlx;
        float frame[THRMLSENS_WIDTH*THRMLSENS_HEIGHT];
        void begin();

        inline void read() { if (mlx.getFrame(frame) != 0) { Serial.println("FF"); return; } }
        inline float temp_at(uint8_t x, uint8_t y) { return frame[y * THRMLSENS_WIDTH + x]; }

        inline void to_image(image_t image) { for (size_t p = 0; p < THRMLSENS_WIDTH * THRMLSENS_HEIGHT; p++) { image.frame[p] = frame[p]; } }
};

inline thermal_sensor_t thermal_sensor;