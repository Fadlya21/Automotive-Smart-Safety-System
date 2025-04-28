#include "tm4c123gh6pm.h"
#include "ultrasonic.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

// HC-SR04 pin definitions (using PB6 for TRIG, PB7 for ECHO)
#define TRIG_PIN    (1 << 6)     // PB6
#define ECHO_PIN    (1 << 7)     // PB7

// Timer configuration for measuring echo pulse
#define TIMER_CLK   16000000     // 16 MHz system clock
#define SOUND_SPEED 343          // Speed of sound in m/s

// Global variable to store the latest distance measurement
static volatile uint16_t g_distance = 0;

/**
 * @brief Initialize HC-SR04 ultrasonic sensor
 */
void Ultrasonic_Init(void)
{
    // Enable clock for Port B
    SYSCTL->RCGCGPIO |= 0x02;    // Enable Port B clock
    
    // Wait for peripheral to be ready
    volatile uint32_t delay = SYSCTL->RCGCGPIO;
    
    // Configure PB6 (TRIG) as output and PB7 (ECHO) as input
    GPIOB->DIR |= TRIG_PIN;      // Set TRIG as output
    GPIOB->DIR &= ~ECHO_PIN;     // Set ECHO as input
    GPIOB->DEN |= (TRIG_PIN | ECHO_PIN); // Enable digital I/O
    
    // Initialize TRIG pin to low
    GPIOB->DATA &= ~TRIG_PIN;
    
    // Enable clock for Timer0
    SYSCTL->RCGCTIMER |= 0x01;   // Enable Timer0 clock
    
    // Wait for peripheral to be ready
    delay = SYSCTL->RCGCTIMER;
    
    // Configure Timer0 for edge time capture
    TIMER0->CTL &= ~0x01;        // Disable Timer0A during configuration
    TIMER0->CFG = 0x04;          // 16-bit timer configuration
    TIMER0->TAMR = 0x17;         // Capture mode, edge-time, up-count
    TIMER0->TAILR = 0xFFFF;      // Maximum count value
    TIMER0->TAPR = 0xFF;         // Use maximum prescaler for long timeouts
    TIMER0->CTL |= 0x0C;         // Capture both edges
    TIMER0->ICR = 0x01;          // Clear capture interrupt flag
    TIMER0->IMR |= 0x01;         // Enable capture interrupt
    
    // Initialize distance to an invalid value
    g_distance = 0;
}

/**
 * @brief Trigger a distance measurement
 */
void Ultrasonic_Trigger(void)
{
    // Variables for timing
    uint32_t startTime = 0;
    uint32_t endTime = 0;
    uint32_t pulseWidth = 0;
    
    // Generate 10µs trigger pulse
    GPIOB->DATA |= TRIG_PIN;     // Set TRIG high
    vTaskDelay(1);               // Wait ~10us (minimum 1 RTOS tick)
    GPIOB->DATA &= ~TRIG_PIN;    // Set TRIG low
    
    // Wait for ECHO pin to go high (start of echo pulse)
    while((GPIOB->DATA & ECHO_PIN) == 0) {
        // Add timeout in case sensor doesn't respond
        if(++startTime > 1000000) {
            g_distance = 0;      // Invalid reading
            return;
        }
    }
    
    // Reset Timer0 counter
    TIMER0->CTL |= 0x01;         // Enable Timer0A
    TIMER0->TAV = 0;             // Reset counter
    
    // Wait for ECHO pin to go low (end of echo pulse)
    while((GPIOB->DATA & ECHO_PIN) != 0) {
        // Add timeout for safety - max echo time is ~38ms (for 400cm range)
        if(++endTime > 4000000) {
            TIMER0->CTL &= ~0x01; // Disable Timer0A
            g_distance = 400;    // Max distance (400cm)
            return;
        }
    }
    
    // Get pulse width from timer
    pulseWidth = TIMER0->TAR;    // Get captured time
    TIMER0->CTL &= ~0x01;        // Disable Timer0A
    
    // Convert time to distance (in cm)
    // Distance = (Time * Speed of sound) / 2
    // Time = pulseWidth / TIMER_CLK (in seconds)
    // Speed of sound = 343 m/s = 34300 cm/s
    // Factor of 2 because sound travels to object and back
    g_distance = (uint16_t)((pulseWidth * 34300) / (TIMER_CLK * 2));
    
    // Limit reading to maximum range (typically 400cm for HC-SR04)
    if(g_distance > 400) {
        g_distance = 400;
    }
}

/**
 * @brief Get the last measured distance
 * @return Distance in centimeters
 */
uint16_t Ultrasonic_GetDistance(void)
{
    return g_distance;
}

/**
 * @brief Get the current zone based on measured distance
 * @return ZONE_SAFE, ZONE_CAUTION, or ZONE_DANGER
 */
uint8_t Ultrasonic_GetZone(void)
{
    if(g_distance > 100) {
        return ZONE_SAFE;      // > 100 cm
    } else if(g_distance > 30) {
        return ZONE_CAUTION;   // 30-100 cm
    } else {
        return ZONE_DANGER;    // < 30 cm
    }
}