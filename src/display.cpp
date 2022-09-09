#include "display.h"

void display_begin() {
  digitalWrite(TFT_BACKLIGHT, HIGH);
  digitalWrite(TFT_RESET, HIGH);
  delay(10);
  digitalWrite(TFT_RESET, LOW);
  delay(10);
  digitalWrite(TFT_RESET, HIGH);
  delay(10);

  display.begin();
  display.setRotation(3);
}