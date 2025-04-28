#ifndef ALERTS_H
#define ALERTS_H

#include <stdint.h>
#include <stdbool.h>

// Distance zone definitions
#define ZONE_SAFE      0   // > 100 cm
#define ZONE_CAUTION   1   // 30-100 cm
#define ZONE_DANGER    2   // < 30 cm

/**
 * @brief Initialize alerts system (buzzer and RGB LED)
 */
void Alerts_Init(void);

/**
 * @brief Set RGB LED color based on distance zone
 * @param zone ZONE_SAFE, ZONE_CAUTION, or ZONE_DANGER
 */
void Alerts_SetLED(uint8_t zone);

/**
 * @brief Start door open warning buzzer
 */
void Alerts_DoorOpenWarning(bool enable);

/**
 * @brief Set proximity buzzer speed based on distance
 * @param distance Distance in centimeters
 */
void Alerts_SetProximityBuzzer(uint16_t distance);

#endif // ALERTS_H