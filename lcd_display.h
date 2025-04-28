#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize LCD display
 */
void LCD_Init(void);

/**
 * @brief Clear LCD display
 */
void LCD_Clear(void);

/**
 * @brief Display door status
 * @param locked True if doors are locked, false otherwise
 */
void LCD_DisplayDoorStatus(bool locked);

/**
 * @brief Display current speed
 * @param speed Speed in km/h
 */
void LCD_DisplaySpeed(uint8_t speed);

/**
 * @brief Display door open warning
 */
void LCD_DisplayDoorOpenWarning(void);

/**
 * @brief Display distance measurement
 * @param distance Distance in centimeters
 */
void LCD_DisplayDistance(uint16_t distance);

#endif // LCD_DISPLAY_H