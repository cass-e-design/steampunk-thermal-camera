#pragma once

//images are stored uncompressed, full-length.
//size: file signature + 24x32 (32bit floats) 

#include <Arduino.h>

//Signature of cTHRMv1 format
inline const uint8_t THRML_IMG_SIG_V1[] = { 0x43, 0x54, 0x48, 0x52, 0x4D, 0x76, 0x31, };

//length of THRML_IMG_FRAME_LEN + THRML_IMG_SIG_V1 LENGTH
#define THRML_IMG_LEN 24583
#define THRML_IMG_FRAME_LEN 24576
#define THRML_IMG_PX 768
#define THRML_IMG_W 32
#define THRML_IMG_H 24

class image_t {
    public:
        float frame[THRML_IMG_PX];

        inline uint8_t width() { return THRML_IMG_W; }
        inline uint8_t height() { return THRML_IMG_H; }

        inline size_t pixelIndex(uint8_t x, uint8_t y) { return y*width()+x; }
        inline float getPixel(uint8_t x, uint8_t y) { return frame[pixelIndex(x, y)]; }
        inline void setPixel(uint8_t x, uint8_t y, float p) { frame[pixelIndex(x, y)] = p; }
};