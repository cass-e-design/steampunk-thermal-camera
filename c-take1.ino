#include <Adafruit_GFX.h>
#include <Adafruit_SPIFlash.h>
#include "Adafruit_HX8357.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_MLX90640.h"
#include "TouchScreen.h"

using u8 = uint8_t;

#define RED_LED       13
#define TFT_RESET     24
#define TFT_BACKLIGHT 25
#define LIGHT_SENSOR  A2
#define SD_CS         32
#define SPKR_SHUTDOWN 50
#define LOW_BATTERY    3

#define TFT_D0        34 // Data bit 0 pin (MUST be on PORT byte boundary)
#define TFT_WR        26 // Write-strobe pin (CCL-inverted timer output)
#define TFT_DC        10 // Data/command pin
#define TFT_CS        11 // Chip-select pin
#define TFT_RST       24 // Reset pin
#define TFT_RD         9 // Read-strobe pin
#define TFT_BACKLIGHT 25

//HX8357 with 8bit parallel interface:+
Adafruit_HX8357 tft = Adafruit_HX8357(tft8bitbus, TFT_D0, TFT_WR, TFT_DC, TFT_CS, TFT_RST, TFT_RD);
#define TFT_W 480
#define TFT_H 320

#define YP A4  // must be an analog pin, use "An" notation!
#define XM A7  // must be an analog pin, use "An" notation!
#define YM A6   // can be a digital pin
#define XP A5   // can be a digital pin
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
#define X_MIN  750
#define X_MAX  325
#define Y_MIN  840
#define Y_MAX  240
u8 previous_pressure = 0;

//MLX
Adafruit_MLX90640 mlx;
#define MLX_WIDTH 32
#define MLX_HEIGHT 24
float mlx_frame[MLX_WIDTH*MLX_HEIGHT];

#define MLX_TEMP_MAX 50 //high point on screen
#define MLX_TEMP_LOW 30 //low temp on screen
#define MLX_ULTRA_TEMP 100 //temps higher than this are pink, flashing?

#define READ_MLX_EVERY 200
size_t last_read_mlx = 0;

//allow us to redraw the MLX display only every other frame, to make the flickering less obvious? 
bool odd_frame;

bool low_battery;

bool freeze_frame;

//readouts for current values on screen
float temp_max;
float temp_mid;
float temp_min;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  pinMode(RED_LED, OUTPUT);
  pinMode(TFT_BACKLIGHT, OUTPUT);
  pinMode(TFT_RESET, OUTPUT);
  
  pinMode(LOW_BATTERY, INPUT_PULLUP);

  //TFT
  digitalWrite(TFT_BACKLIGHT, HIGH);

  digitalWrite(TFT_RESET, HIGH);
  delay(10);
  digitalWrite(TFT_RESET, LOW);
  delay(10);
  digitalWrite(TFT_RESET, HIGH);
  delay(10);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(HX8357_BLACK);

  //mlx
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
    tft.setTextColor(HX8357_RED);
    tft.println("MLX90640 not found!");
    while (1) delay(10); 
  }
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_4_HZ);
}

//uint16_t interpolate(uint8_t x_orig, uint8_t y_orig) {

//}
//https://forum.arduino.cc/t/adafruit-library-tft-colours/239777/7
uint16_t rgb(u8 R, u8 G, u8 B)
{
  return ( ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3) );
}

//https://stackoverflow.com/questions/48073159/most-efficient-linear-interpolation-using-integer-fraction
u8 lerp(u8 a, u8 b, u8 t) {
  return a + (uint8_t)(((int16_t)(b-a) * t) >> 8);
}
uint16_t lerp_rgb(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2, u8 t) { return rgb(lerp(r1, r2, t), lerp(g1, g2, t), lerp(b1, b2, t)); }
uint16_t color_temp(u8 temp1, u8 temp2, u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2, float temp) {
  float t = (float)(temp - temp1) / (float)(temp2 - temp1);
  u8 a = (u8)(int)(t * 255);
  return lerp_rgb(r1, g1, b1, r2, g2, b2, a); 
}

uint16_t heatmap(float temp) {
  if (temp < 20) {
    return color_temp( 0, 20,   000,000,000,   000,000,060, temp);
  } else if (temp < 30) {
    return color_temp(20, 30,   000,000,060,   200,040,000, temp);
  } else if (temp < 40) {
    return color_temp(30, 40,   200,040,000,   255,196,030, temp);
  } else if (temp < 60) {
    return color_temp(40, 60,   255,196,030,   255,255,000, temp);
  } else if (temp < 70) {
    return color_temp(60, 70,   255,255,000,   255,123,172, temp);
  } else if (temp < MLX_ULTRA_TEMP)
    return color_temp(70,100,   255,123,172 ,   255,255,255, temp);
  else 
    return ILI9341_PINK;
  
  //u8 grey = (u8)(constrain((temp - MLX_TEMP_LOW) / (MLX_TEMP_MAX-MLX_TEMP_LOW), 0, 1) * 255 + 0.5);
  //return rgb(grey, grey, grey);
}

void draw_frame(u8 radius, uint16_t color) {
    tft.fillRect(           0,           0, radius, TFT_H, color);
    tft.fillRect(           0,           0,  TFT_W,radius, color);
    tft.fillRect(TFT_W-radius,           0, radius, TFT_H, color);
    tft.fillRect(           0,TFT_H-radius,  TFT_W,radius, color);
}

bool checkerboard(bool odd, uint8_t x, uint8_t y) { return ((odd && x % 2 != y % 2) || (!odd && x % 2 == y % 2)); }

void loop() {
  delay(100);
  odd_frame = !odd_frame;
  
  //const uint8_t x_scale = (TFT_W-6)/MLX_HEIGHT;
  const uint8_t y_scale = ((TFT_H-6)/MLX_WIDTH)+1;
  const uint8_t x_scale = y_scale * 1.35;

  const uint8_t sub_w_s = x_scale / 2;
  const uint8_t sub_h_s = y_scale / 2 + 1;

  //don't refresh frame during freeze_frame
  if (!freeze_frame && millis() - last_read_mlx > READ_MLX_EVERY) {
    if (mlx.getFrame(mlx_frame) != 0) { Serial.println("FF"); return; }
    last_read_mlx = millis();
    Serial.println("Read MLX");
  }

  low_battery = !digitalRead(LOW_BATTERY);

  temp_max = 0;
  temp_mid = 0;
  temp_min = 1000;

  for (uint8_t h = 0; h < MLX_HEIGHT; h++) {
    for (uint8_t w = 0; w < MLX_WIDTH; w++) {
      //checkerboard interlace
      if (checkerboard(odd_frame, w, h)) continue;

      if (h < MLX_HEIGHT - 1 && w < MLX_WIDTH - 1) {
        float tl = mlx_frame[h*MLX_WIDTH+w];
        float tr = mlx_frame[h*MLX_WIDTH+(w+1)];
        float bl = mlx_frame[(h+1)*MLX_WIDTH+w];
        float br = mlx_frame[(h+1)*MLX_WIDTH+(w+1)];

        /*
        Serial.print("Rendering subpixels @");
        Serial.print(w);
        Serial.print(", ");
        Serial.print(h);
        Seria.println();
        Serial.flush();
        */

        for (uint8_t sub_h = 0; sub_h < x_scale; sub_h+=sub_h_s) {
          for (uint8_t sub_w = 0; sub_w < y_scale; sub_w+=sub_w_s) {

            float xratio = 1.0F-(float)sub_w/((float)x_scale-1);
            float yratio = 1.0F-(float)sub_h/((float)y_scale-1);

            float t = tl*      xratio  + tl*      yratio
                    + tr*(1.0F-xratio) + tr*      yratio
                    + bl*      xratio  + bl*(1.0F-yratio)
                    + br*(1.0F-xratio) + br*(1.0F-yratio);
            t /= 4.0F;

            if (t > temp_max) {
              temp_max = t;
            }
            if (t < temp_min) { 
              temp_min = t;
            }

            //tft.drawPixel(w*x_scale + sub_w, h*y_scale + sub_h, heatmap(t));
            tft.fillRect((TFT_W-170) - (h*x_scale + sub_h + sub_w_s), (TFT_H-8) - (w*y_scale + sub_w), sub_w_s, sub_h_s, heatmap(t));
              
          }
        }

      } else if (odd_frame || true) {
        float t = mlx_frame[h*MLX_WIDTH+w];
        tft.fillRect((TFT_W-170) - (h*x_scale + x_scale), (TFT_H-8) - (w*y_scale), x_scale, y_scale, heatmap(t));
        //Serial.println("Cannot interpolate end-tiles");
      }
    }
  }
  temp_mid = (temp_min + temp_max) / 2;



  //PROCESS INPUT
  TSPoint touch_point = ts.getPoint();
  //if current touch
  if (touch_point.z >= 1) {
    tft.drawPixel(touch_point.x, touch_point.y, (uint16_t)0xFFFF00);
  }

  //if new press
  if (previous_pressure == 0 && touch_point.z >= 1) {
    freeze_frame = !freeze_frame;
  }
  previous_pressure = touch_point.z;

  if (freeze_frame) {
    draw_frame(3, (uint16_t)0x5050FF);
  } else {
    draw_frame(3, 0);
  }
  
  if (low_battery) {
    draw_frame(10, 0xF800);
    tft.setCursor(20, TFT_H-25);
    tft.setTextSize(2);
    tft.print("BATTERY LOW");
  }

  //DRAW UI (top bar)
  tft.setTextSize(2);

  tft.setCursor(330,115);
  tft.fillRect(310,100, 160,40, heatmap(temp_max));
  tft.setTextColor(temp_max > 40 ? 0 : 0xFFF);
  tft.print("MAX:");
  tft.print(temp_max);
  tft.print("C");

  tft.setCursor(330,155);
  tft.setTextColor(0xFFFF);
  tft.fillRect(310,140, 160,40, heatmap(temp_mid));
  tft.print("AVG:");
  tft.print(temp_mid);
  tft.print("C");

  tft.setCursor(330,195);
  tft.fillRect(310,180, 160,40, heatmap(temp_min));
  tft.setTextColor(0xFFFF);
  tft.print("MIN:");
  tft.print(temp_min);
  tft.print("C");

  tft.setTextColor(ILI9341_DARKGREY);
  tft.setCursor(TFT_W-160,230);
  tft.print("Cassia Duske");
}