#ifndef ADC_SPEED_H
#define ADC_SPEED_H

#include <stdint.h>

/**
 * @brief Initialize ADC for potentiometer speed reading
 */
void SpeedSensor_Init(void);

/**
 * @brief Read current speed from potentiometer
 * @return Speed value in km/h (0-100)
 */
uint8_t ReadSpeed(void);

#endif // ADC_SPEED_H