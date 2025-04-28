#include "tm4c123gh6pm.h"
#include "alerts.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdbool.h>

// Define pins for alerts
#define BUZZER_PIN          (1 << 2)    // PC2 for buzzer
// RGB LED pins on Port F
#define LED_RED_PIN         (1 << 1)    // PF1 for red LED
#define LED_GREEN_PIN       (1 << 3)    // PF3 for green LED
#define LED_BLUE_PIN        (1 << 2)    // PF2 for blue LED
#define LED_ALL_PINS        (LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN)

// Buzzer control variables
static bool doorWarningEnabled = false;
static uint16_t proximityDistance = 0;
static TickType_t buzzerToneFrequency = 0;
static TaskHandle_t xBuzzerTask = NULL;

// Buzzer task declaration
static void vBuzzerTask(void *pvParameters);

/**
 * @brief Initialize alerts system (buzzer and RGB LED)
 */
void Alerts_Init(void)
{
    // Enable clocks for Port C and Port F
    SYSCTL->RCGCGPIO |= 0x24;    // Enable Port C and F clocks
    
    // Wait for the peripherals to be ready
    volatile uint32_t delay = SYSCTL->RCGCGPIO;
    
    // Configure buzzer pin (PC2)
    GPIOC->DIR |= BUZZER_PIN;    // Set as output
    GPIOC->DEN |= BUZZER_PIN;    // Enable digital I/O
    GPIOC->DATA &= ~BUZZER_PIN;  // Initialize to low
    
    // Configure RGB LED pins (PF1, PF2, PF3)
    GPIOF->DIR |= LED_ALL_PINS;  // Set as outputs
    GPIOF->DEN |= LED_ALL_PINS;  // Enable digital I/O
    GPIOF->DATA &= ~LED_ALL_PINS; // Initialize all LEDs to off
    
    // Create a task to control the buzzer
    xTaskCreate(vBuzzerTask, "Buzzer", configMINIMAL_STACK_SIZE, NULL, 1, &xBuzzerTask);
}

/**
 * @brief Set RGB LED color based on distance zone
 * @param zone ZONE_SAFE, ZONE_CAUTION, or ZONE_DANGER
 */
void Alerts_SetLED(uint8_t zone)
{
    // Turn off all LEDs first
    GPIOF->DATA &= ~LED_ALL_PINS;
    
    // Set color based on zone
    switch(zone) {
        case ZONE_SAFE:
            GPIOF->DATA |= LED_GREEN_PIN;  // Green for safe
            break;
        case ZONE_CAUTION:
            GPIOF->DATA |= LED_GREEN_PIN | LED_RED_PIN;  // Yellow (Red + Green) for caution
            break;
        case ZONE_DANGER:
            GPIOF->DATA |= LED_RED_PIN;    // Red for danger
            break;
        default:
            // Default to all off
            break;
    }
}

/**
 * @brief Start door open warning buzzer
 * @param enable True to enable warning, false to disable
 */
void Alerts_DoorOpenWarning(bool enable)
{
    doorWarningEnabled = enable;
}

/**
 * @brief Set proximity buzzer speed based on distance
 * @param distance Distance in centimeters
 */
void Alerts_SetProximityBuzzer(uint16_t distance)
{
    proximityDistance = distance;
    
    // Calculate buzzer tone frequency based on distance
    if(distance == 0) {
        // Disable buzzer if distance is invalid or proximity detection is off
        buzzerToneFrequency = 0;
    } else if(distance < 30) {
        // Constant tone for very close objects
        buzzerToneFrequency = 50;     // Fast beeping (50ms on, 50ms off)
    } else if(distance < 100) {
        // Map distance 30-100cm to buzzer frequency 50-500ms
        buzzerToneFrequency = 50 + ((distance - 30) * 450) / 70;
    } else {
        // Disable buzzer for safe distances
        buzzerToneFrequency = 0;
    }
}

/**
 * @brief Buzzer control task
 * @param pvParameters Task parameters (not used)
 */
static void vBuzzerTask(void *pvParameters)
{
    bool buzzerState = false;
    TickType_t xLastWakeTime;
    const TickType_t xDoorWarningFrequency = pdMS_TO_TICKS(500);  // 500ms for door warning
    
    // Initialize xLastWakeTime
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;) {
        // Check if door warning is enabled
        if(doorWarningEnabled) {
            // Toggle buzzer for door warning (500ms on, 500ms off)
            buzzerState = !buzzerState;
            GPIOC->DATA = buzzerState ? BUZZER_PIN : 0;
            vTaskDelayUntil(&xLastWakeTime, xDoorWarningFrequency);
        }
        // Check if proximity buzzer is enabled
        else if(buzzerToneFrequency > 0) {
            // Toggle buzzer state
            buzzerState = !buzzerState;
            GPIOC->DATA = buzzerState ? BUZZER_PIN : 0;
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(buzzerToneFrequency));
        }
        // If no alerts active, ensure buzzer is off
        else {
            GPIOC->DATA &= ~BUZZER_PIN;  // Buzzer off
            buzzerState = false;
            vTaskDelay(pdMS_TO_TICKS(100));  // Check again after 100ms
            xLastWakeTime = xTaskGetTickCount();  // Reset wake time
        }
    }
}