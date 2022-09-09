#include <Arduino.h>

#include "pins.h"
#include "apps.h"
#include "display.h"
#include "thermal_sensor.h"
#include "touch.h"
#include "storage.h"

using u8 = uint8_t;

state_t state;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  //necessary pins
  pinMode(RED_LED, OUTPUT);
  pinMode(TFT_BACKLIGHT, OUTPUT);
  pinMode(TFT_RESET, OUTPUT);
  
  //battery check
  pinMode(LOW_BATTERY, INPUT_PULLUP);

  //TFT setup
  display_begin();
  display.fillScreen(HX8357_BLACK);

  //storage
  storage.begin();

  //mlx
  thermal_sensor.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);

  state.battery_low = !digitalRead(LOW_BATTERY);
  
  //PROCESS INPUT
  touch.update();
  if (touch.touch) {
    display.drawPixel(touch.current_touch.x, touch.current_touch.y, (uint16_t)0xFFFF00);
  }

  //update current app! It will redraw if needed
  apps[state.app]->update(state);
  
  if (state.battery_low) {
    draw_border(10, 0xF800);
    display.setCursor(20, DISPLAY_WIDTH-25);
    display.setTextSize(2);
    display.print("BATTERY LOW");
  }
  
  display.setTextColor(ILI9341_DARKGREY);
  display.setCursor(DISPLAY_WIDTH-150,DISPLAY_HEIGHT-20);
  display.print("Cassia Duske");
}