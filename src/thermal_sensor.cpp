#include "thermal_sensor.h"
#include "display.h"

void thermal_sensor_t::begin() {
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
    display.setTextColor(HX8357_RED);
    display.println("MLX90640 not found!");
    while (1) delay(10); 
  }
  
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_4_HZ);
}