#include "app_camera.h"
#include "thermal_sensor.h"
#include "display.h"
#include "touch.h"
#include "falsecolor.h"
#include "storage.h"
#include "image.h"

uint16_t color_temp(uint8_t temp1, uint8_t temp2, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float temp) {
  float t = (float)(temp - temp1) / (float)(temp2 - temp1);
  uint8_t a = (uint8_t)(int)(t * 255);
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
  
  //uint8_t grey = (uint8_t)(constrain((temp - MLX_TEMP_LOW) / (MLX_TEMP_MAX-MLX_TEMP_LOW), 0, 1) * 255 + 0.5);
  //return rgb(grey, grey, grey);
}


#define THRML_OFFSET_X 3
#define THRML_OFFSET_Y 3

//Height of an "MLX Pixel" on display
#define PX_MLX_Y (((DISPLAY_HEIGHT-6)/THRMLSENS_WIDTH)+1)
//Width of an "MLX Pixel" on display
#define PX_MLX_X (PX_MLX_Y + 3)

//Number of super-sampled pixels per "MLX Pixel" in Y
#define PXS_SUP_Y 3
//Number of super-sampled pixels per "MLX Pixel" in X
#define PXS_SUP_X 3

//Width of an "MLX Pixel" on display
#define PX_SUP_Y ((PX_MLX_Y / PXS_SUP_Y)+1)
//Width of a super-sampled pixel on display
#define PX_SUP_X ((PX_MLX_X / PXS_SUP_X))

void draw_thrml_px(uint16_t x, uint16_t y, uint16_t x_subPxl, uint16_t y_subPxl, float temp) {
  x = THRMLSENS_WIDTH  - x - 1;
  y = THRMLSENS_HEIGHT - y - 1;

  // this will auto-fill any gaps / auto-shrink overap
  // by enlarging the last pixel of each supersampled set. 
  // Possibly noticeable, but reduces wiggle!
  const uint8_t width  = (y_subPxl < (PXS_SUP_X - 1)) ? PX_SUP_X : (PX_MLX_X-(PX_SUP_X*(PXS_SUP_X-1)));
  const uint8_t height = (x_subPxl < (PXS_SUP_Y - 1)) ? PX_SUP_Y : (PX_MLX_Y-(PX_SUP_Y*(PXS_SUP_Y-1)));

  display.fillRect(THRML_OFFSET_X + (y*PX_MLX_X) + (y_subPxl*PX_SUP_X), 
                   THRML_OFFSET_Y + (x*PX_MLX_Y) + (x_subPxl*PX_SUP_Y), 
                   width, 
                   height, 
                   heatmap(temp));
}

void draw_thrml_frame(bool odd_frame) {
  for (uint8_t y = 0; y < THRMLSENS_HEIGHT; y++) {
    for (uint8_t x = 0; x < THRMLSENS_WIDTH; x++) {
      //checkerboard interlace
      if (checkerboard(odd_frame, x, y)) continue;

      if (y < THRMLSENS_HEIGHT - 1 && x < THRMLSENS_WIDTH - 1) {
        float br = thermal_sensor.temp_at(x  , y  );
        float tr = thermal_sensor.temp_at(x  , y+1);
        float bl = thermal_sensor.temp_at(x+1, y  );
        float tl = thermal_sensor.temp_at(x+1, y+1);

        for (uint8_t y_subPxl = 0; y_subPxl < PXS_SUP_Y; y_subPxl++) {
          for (uint8_t x_subPxl = 0; x_subPxl < PXS_SUP_X; x_subPxl++) {
            float xratio = 1.0F-(float)x_subPxl/((float)PXS_SUP_X-1);
            float yratio = 1.0F-(float)y_subPxl/((float)PXS_SUP_Y-1);

            float t = tl*      xratio  + tl*      yratio
                    + tr*(1.0F-xratio) + tr*      yratio
                    + bl*      xratio  + bl*(1.0F-yratio)
                    + br*(1.0F-xratio) + br*(1.0F-yratio);
            t /= 4.0F;

            draw_thrml_px(x, y, x_subPxl, y_subPxl, t);
          }
        }
      } else {
        float t = thermal_sensor.temp_at(x, y);
        for (uint8_t y_subPxl = 0; y_subPxl < PXS_SUP_Y; y_subPxl++) {
          for (uint8_t x_subPxl = 0; x_subPxl < PXS_SUP_X; x_subPxl++) {
            draw_thrml_px(x, y, x_subPxl, y_subPxl, t);
          }
        }
        //Serial.println("Cannot interpolate end-tiles");
      }
    }
  }
}


#define SIDEBAR_W 165
#define SIDEBAR_X (DISPLAY_WIDTH - SIDEBAR_W)

void app_camera::draw() {
  app_t::draw();

  odd_frame = !odd_frame;
  
  draw_thrml_frame(odd_frame);
  
  if (freeze_frame) {
    draw_border(3, (uint16_t)0x5050FF);
  } else {
    draw_border(3, 0);
  }
  
  //DRAW UI (top bar)
  display.setTextSize(2);

  display.setCursor(SIDEBAR_X+20,115);
  display.fillRect(SIDEBAR_X,100, SIDEBAR_W,40, heatmap(temp_max));
  display.setTextColor(temp_max > 40 ? 0 : 0xFFF);
  display.print("MAX:");
  display.print(temp_max);
  display.print("C");

  display.setCursor(SIDEBAR_X+20,155);
  display.setTextColor(0xFFFF);
  display.fillRect(SIDEBAR_X,140, SIDEBAR_W,40, heatmap(temp_mid));
  display.print("AVG:");
  display.print(temp_mid);
  display.print("C");

  display.setCursor(SIDEBAR_X+20,195);
  display.fillRect(SIDEBAR_X,180, SIDEBAR_W,40, heatmap(temp_min));
  display.setTextColor(0xFFFF);
  display.print("MIN:");
  display.print(temp_min);
  display.print("C");

  //TODO: Draw SD card presence icon + storage capacity
  if (storage.available) {
    display.drawRect(SIDEBAR_X+10,10, SIDEBAR_W-20,16, ILI9341_WHITE);
    //TODO: draw icon
    display.setTextColor(ILI9341_WHITE);
    display.setCursor(SIDEBAR_X+40, 12);
    display.print("GALLERY");
  } else {
    display.setTextColor(ILI9341_WHITE);
    display.setCursor(SIDEBAR_X+30,12);
    display.print("NO STORAGE");
  }
  storage.bytes_available(THRML_IMG_LEN);
};

void app_camera::update(state_t state) {
  //don't refresh frame during freeze_frame
  if (!freeze_frame && millis() - last_read_mlx > READ_MLX_EVERY) {
    thermal_sensor.read();
    last_read_mlx = millis();
    Serial.println("Read MLX");
  }
  
  temp_max = 0;
  temp_mid = 0;
  temp_min = 1000;

  //doing this in update instead of draw to reduce complexity, but it does mean another iteration...
  for (uint8_t thrml_y = 0; thrml_y < THRMLSENS_HEIGHT; thrml_y++ ) {
    for (uint8_t thrml_x = 0; thrml_x < THRMLSENS_HEIGHT; thrml_x++ ) {
      const float t = thermal_sensor.temp_at(thrml_x, thrml_y);
      if (t > temp_max) { temp_max = t; }
      else if (t < temp_min) { temp_min = t; }
    }
  }
  temp_mid = (temp_max + temp_min) / 2.0F;

  if (!freeze_frame) redraw_needed = true;

  if (touch.new_touch) {
    freeze_frame = !freeze_frame;

    //TODO: trigger picture animation
    if (storage.bytes_available(THRML_IMG_LEN)) {
      thermal_sensor.to_image(image_buffer);
      storage.writeImage(image_buffer);
    }
  }
  
  if (!freeze_frame) redraw_needed = true;

  app_t::update(state);
};