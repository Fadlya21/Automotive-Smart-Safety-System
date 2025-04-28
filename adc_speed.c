#include "tm4c123gh6pm.h"
#include "adc_speed.h"
#include <stdint.h>

// ADC configuration definitions
#define ADC_POTENTIOMETER        ADC0 // Using ADC0
#define POTENTIOMETER_CHANNEL    1    // Using AIN1 (PE2)

/**
 * @brief Initialize ADC for potentiometer speed reading
 */
void SpeedSensor_Init(void)
{
    // Enable clock for ADC0 and Port E
    SYSCTL->RCGCADC |= 0x01;     // Enable ADC0 clock
    SYSCTL->RCGCGPIO |= 0x10;    // Enable Port E clock
    
    // Wait for the peripherals to be ready
    volatile uint32_t delay = SYSCTL->RCGCGPIO;
    
    // Configure PE2 for analog input
    GPIOE->AFSEL |= (1 << 2);    // Enable alternate function on PE2
    GPIOE->DEN &= ~(1 << 2);     // Disable digital function on PE2
    GPIOE->AMSEL |= (1 << 2);    // Enable analog function on PE2
    
    // Configure ADC0
    ADC0->ACTSS &= ~(1 << 3);    // Disable SS3 during configuration
    ADC0->EMUX &= ~0xF000;       // Software trigger for SS3
    ADC0->SSMUX3 = POTENTIOMETER_CHANNEL; // Set channel for AIN1 (PE2)
    ADC0->SSCTL3 |= 0x06;        // Set IE0 and END0 bits
    ADC0->PC = 0x1;              // Configure for 125k samples/sec
    ADC0->ACTSS |= (1 << 3);     // Enable SS3
}

/**
 * @brief Read current speed from potentiometer
 * @return Speed value in km/h (0-100)
 */
uint8_t ReadSpeed(void)
{
    uint32_t result;
    
    // Trigger conversion
    ADC0->PSSI |= (1 << 3);      // Start conversion on SS3
    
    // Wait for conversion to complete
    while((ADC0->RIS & (1 << 3)) == 0);
    
    // Read result
    result = ADC0->SSFIFO3;      // Read conversion result
    
    // Clear completion flag
    ADC0->ISC |= (1 << 3);
    
    // Scale from 12-bit ADC result (0-4095) to speed in km/h (0-100)
    return (uint8_t)((result * 100) / 4096);
}