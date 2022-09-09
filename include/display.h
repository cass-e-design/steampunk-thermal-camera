#pragma once

#include <Adafruit_HX8357.h>
#include <Adafruit_ILI9341.h>

#include "pins.h"

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320

//HX8357 with 8bit parallel interface:
inline Adafruit_HX8357 display(tft8bitbus, TFT_D0, TFT_WR, TFT_DC, TFT_CS, TFT_RST, TFT_RD);

void display_begin();

//https://forum.arduino.cc/t/adafruit-library-tft-colours/239777/7
inline uint16_t rgb(uint8_t R, uint8_t G, uint8_t B) { return ( ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3) ); }

//https://stackoverflow.com/questions/48073159/most-efficient-linear-interpolation-using-integer-fraction
inline uint8_t lerp(uint8_t a, uint8_t b, uint8_t t) { return a + (uint8_t)(((int16_t)(b-a) * t) >> 8); }

inline uint16_t lerp_rgb(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t t) { return rgb(lerp(r1, r2, t), lerp(g1, g2, t), lerp(b1, b2, t)); }

inline void draw_border(uint8_t radius, uint16_t color) {
    display.fillRect(                   0,                     0,         radius, DISPLAY_HEIGHT, color);
    display.fillRect(                   0,                     0,  DISPLAY_WIDTH,         radius, color);
    display.fillRect(DISPLAY_WIDTH-radius,                     0,         radius, DISPLAY_HEIGHT, color);
    display.fillRect(                   0, DISPLAY_HEIGHT-radius,  DISPLAY_WIDTH,         radius, color);
}

inline bool checkerboard(bool odd, uint8_t x, uint8_t y) { return ((odd && x % 2 != y % 2) || (!odd && x % 2 == y % 2)); }