#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

/**
 * @brief Initialize HC-SR04 ultrasonic sensor
 */
void Ultrasonic_Init(void);

/**
 * @brief Trigger a distance measurement
 */
void Ultrasonic_Trigger(void);

/**
 * @brief Get the last measured distance
 * @return Distance in centimeters
 */
uint16_t Ultrasonic_GetDistance(void);

// Distance zones
#define ZONE_SAFE      0   // > 100 cm
#define ZONE_CAUTION   1   // 30-100 cm
#define ZONE_DANGER    2   // < 30 cm

/**
 * @brief Get the current zone based on measured distance
 * @return ZONE_SAFE, ZONE_CAUTION, or ZONE_DANGER
 */
uint8_t Ultrasonic_GetZone(void);

#endif // ULTRASONIC_H