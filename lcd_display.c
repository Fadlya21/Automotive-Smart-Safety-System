#include "tm4c123gh6pm.h"
#include "lcd_display.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// LCD pin definitions (using 4-bit mode)
// Control pins on Port A
#define LCD_RS      (1 << 6)     // PA6
#define LCD_EN      (1 << 7)     // PA7
// Data pins on Port D
#define LCD_D4      (1 << 0)     // PD0
#define LCD_D5      (1 << 1)     // PD1
#define LCD_D6      (1 << 2)     // PD2
#define LCD_D7      (1 << 3)     // PD3
#define LCD_DATA    (LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7) // All data pins

// LCD commands
#define LCD_CLEAR           0x01
#define LCD_HOME            0x02
#define LCD_ENTRY_MODE      0x06
#define LCD_DISPLAY_ON      0x0C
#define LCD_FUNCTION_SET    0x28  // 4-bit mode, 2 lines, 5x8 font
#define LCD_SET_CURSOR      0x80  // Set DDRAM address command

// Delay functions
static void delay_us(uint32_t us);
static void delay_ms(uint32_t ms);

// LCD low-level functions
static void LCD_Command(uint8_t cmd);
static void LCD_Data(uint8_t data);
static void LCD_Write4Bits(uint8_t value);
static void LCD_SetCursor(uint8_t row, uint8_t col);
static void LCD_WriteString(const char* str);

/**
 * @brief Initialize LCD display
 */
void LCD_Init(void)
{
    // Enable clocks for Port A and Port D
    SYSCTL->RCGCGPIO |= 0x09;    // Enable Port A and Port D clocks
    
    // Wait for the peripherals to be ready
    volatile uint32_t delay = SYSCTL->RCGCGPIO;
    
    // Configure Port A pins (control)
    GPIOA->DIR |= (LCD_RS | LCD_EN);    // Set as outputs
    GPIOA->DEN |= (LCD_RS | LCD_EN);    // Enable digital I/O
    
    // Configure Port D pins (data)
    GPIOD->DIR |= LCD_DATA;     // Set as outputs
    GPIOD->DEN |= LCD_DATA;     // Enable digital I/O
    
    // Initialize LCD in 4-bit mode
    delay_ms(50);               // Wait for LCD to power up
    
    // Initialize sequence for 4-bit mode
    GPIOA->DATA &= ~LCD_RS;     // RS = 0 for command
    
    // Special initialization sequence for 4-bit mode
    LCD_Write4Bits(0x3);        // Function set (8-bit mode) - 1st try
    delay_ms(5);
    LCD_Write4Bits(0x3);        // Function set (8-bit mode) - 2nd try
    delay_us(150);
    LCD_Write4Bits(0x3);        // Function set (8-bit mode) - 3rd try
    delay_us(150);
    LCD_Write4Bits(0x2);        // Set 4-bit mode
    delay_us(150);
    
    // Configure display
    LCD_Command(LCD_FUNCTION_SET);  // 4-bit mode, 2 lines
    LCD_Command(LCD_DISPLAY_ON);    // Display on, cursor off, no blink
    LCD_Command(LCD_CLEAR);         // Clear display
    delay_ms(2);
    LCD_Command(LCD_ENTRY_MODE);    // Increment cursor, no display shift
    
    // Clear display and set cursor to home
    LCD_Clear();
}

/**
 * @brief Clear LCD display
 */
void LCD_Clear(void)
{
    LCD_Command(LCD_CLEAR);
    delay_ms(2);  // Clear command needs a longer delay
}

/**
 * @brief Display door status
 * @param locked True if doors are locked, false otherwise
 */
void LCD_DisplayDoorStatus(bool locked)
{
    LCD_SetCursor(0, 0);
    if(locked) {
        LCD_WriteString("Doors: LOCKED   ");
    } else {
        LCD_WriteString("Doors: UNLOCKED ");
    }
}

/**
 * @brief Display current speed
 * @param speed Speed in km/h
 */
void LCD_DisplaySpeed(uint8_t speed)
{
    char buffer[16];
    LCD_SetCursor(1, 0);
    sprintf(buffer, "Speed: %3d km/h", speed);
    LCD_WriteString(buffer);
}

/**
 * @brief Display door open warning
 */
void LCD_DisplayDoorOpenWarning(void)
{
    LCD_SetCursor(0, 0);
    LCD_WriteString("WARNING!        ");
    LCD_SetCursor(1, 0);
    LCD_WriteString("Door is OPEN    ");
    delay_ms(1000);  // Keep warning visible for 1 second
}

/**
 * @brief Display distance measurement
 * @param distance Distance in centimeters
 */
void LCD_DisplayDistance(uint16_t distance)
{
    char buffer[16];
    LCD_SetCursor(0, 0);
    LCD_WriteString("REVERSE GEAR    ");
    LCD_SetCursor(1, 0);
    sprintf(buffer, "Distance: %3d cm", distance);
    LCD_WriteString(buffer);
}

/**
 * @brief Send a command to the LCD
 * @param cmd Command byte
 */
static void LCD_Command(uint8_t cmd)
{
    GPIOA->DATA &= ~LCD_RS;    // RS = 0 for command
    
    // Send upper nibble
    LCD_Write4Bits(cmd >> 4);
    
    // Send lower nibble
    LCD_Write4Bits(cmd & 0x0F);
    
    // If command is clear or return home, delay longer
    if(cmd < 4) {
        delay_ms(2);
    } else {
        delay_us(50);
    }
}

/**
 * @brief Send data to the LCD
 * @param data Data byte
 */
static void LCD_Data(uint8_t data)
{
    GPIOA->DATA |= LCD_RS;     // RS = 1 for data
    
    // Send upper nibble
    LCD_Write4Bits(data >> 4);
    
    // Send lower nibble
    LCD_Write4Bits(data & 0x0F);
    
    delay_us(50);
}

/**
 * @brief Write 4 bits to the LCD
 * @param value 4-bit value to write
 */
static void LCD_Write4Bits(uint8_t value)
{
    // Clear the data pins
    GPIOD->DATA &= ~LCD_DATA;
    
    // Set the data pins according to the value
    if(value & 0x1) GPIOD->DATA |= LCD_D4;
    if(value & 0x2) GPIOD->DATA |= LCD_D5;
    if(value & 0x4) GPIOD->DATA |= LCD_D6;
    if(value & 0x8) GPIOD->DATA |= LCD_D7;
    
    // Toggle the EN pin to latch the data
    GPIOA->DATA |= LCD_EN;
    delay_us(1);
    GPIOA->DATA &= ~LCD_EN;
    delay_us(1);
}

/**
 * @brief Set the cursor position
 * @param row Row (0-1)
 * @param col Column (0-15)
 */
static void LCD_SetCursor(uint8_t row, uint8_t col)
{
    static const uint8_t row_offsets[] = {0x00, 0x40};
    LCD_Command(LCD_SET_CURSOR | (row_offsets[row] + col));
}

/**
 * @brief Write a string to the LCD
 * @param str String to write
 */
static void LCD_WriteString(const char* str)
{
    while(*str) {
        LCD_Data(*str++);
    }
}

/**
 * @brief Microsecond delay function
 * @param us Number of microseconds to delay
 */
static void delay_us(uint32_t us)
{
    // Approximate delay - system clock 16MHz
    // Each cycle is 1/16MHz = 62.5ns
    // Each iteration of the loop takes ~5 cycles
    // 5 * 62.5ns = 312.5ns per iteration
    // us * 1000ns/us / 312.5ns = ~3.2 iterations per us
    volatile uint32_t count = us * 3;
    while(count--);
}

/**
 * @brief Millisecond delay function
 * @param ms Number of milliseconds to delay
 */
static void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms; i++) {
        delay_us(1000);
    }
}