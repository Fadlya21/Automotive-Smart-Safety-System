#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "car_locking.h"

// Define the pins for Car Locking
#define LOCK_BUTTON_PIN         (1 << 4)    // PF4 (Manual Lock Button)
#define UNLOCK_BUTTON_PIN       (1 << 0)    // PF0 (Manual Unlock Button)
#define IGNITION_SWITCH_PIN     (1 << 2)    // PA2 (Ignition Switch)
#define DRIVER_DOOR_SWITCH_PIN  (1 << 3)    // PA3 (Driver Door Switch)
#define GEAR_SWITCH_PIN_0       (1 << 4)    // PB4 (Gear Switch bit 0)
#define GEAR_SWITCH_PIN_1       (1 << 5)    // PB5 (Gear Switch bit 1)
#define GEAR_SWITCH_PINS        (GEAR_SWITCH_PIN_0 | GEAR_SWITCH_PIN_1)

// Define the car lock status pin
#define LOCK_STATUS_PIN         (1 << 1)    // PF1 (LED for Locked Status)

// Locking and Unlocking States
static bool carLocked = false;

// Helper functions to read pin state
static bool ReadPin(volatile uint32_t *port, uint8_t pinMask)
{
    return ((*port & pinMask) != 0);
}

// Helper function to write a value to a pin
static void WritePin(volatile uint32_t *port, uint8_t pinMask, bool value)
{
    if (value)
        *port |= pinMask;
    else
        *port &= ~pinMask;
}

// Lock the doors
void LockDoors(void)
{
    carLocked = true;
    WritePin(&GPIOF->DATA, LOCK_STATUS_PIN, true); // Lock status LED ON
}

// Unlock the doors
void UnlockDoors(void)
{
    carLocked = false;
    WritePin(&GPIOF->DATA, LOCK_STATUS_PIN, false); // Lock status LED OFF
}

// Get the current lock status
bool GetLockStatus(void)
{
    return carLocked;
}

// Check if Manual Lock button is pressed (active low)
bool ManualLockPressed(void)
{
    return !ReadPin(&GPIOF->DATA, LOCK_BUTTON_PIN);
}

// Check if Manual Unlock button is pressed (active low)
bool ManualUnlockPressed(void)
{
    return !ReadPin(&GPIOF->DATA, UNLOCK_BUTTON_PIN);
}

// Read the ignition switch (ON = true, OFF = false)
bool ReadIgnitionSwitch(void)
{
    return ReadPin(&GPIOA->DATA, IGNITION_SWITCH_PIN);
}

// Read the driver door switch (OPEN = true, CLOSED = false)
bool ReadDriverDoorSwitch(void)
{
    return ReadPin(&GPIOA->DATA, DRIVER_DOOR_SWITCH_PIN);
}

// Read the gear switch (PARK = 0, DRIVE = 1, REVERSE = 2)
uint8_t ReadGearSwitch(void)
{
    // Read the two bits from gear switch pins
    uint8_t gearBit0 = ReadPin(&GPIOB->DATA, GEAR_SWITCH_PIN_0) ? 1 : 0;
    uint8_t gearBit1 = ReadPin(&GPIOB->DATA, GEAR_SWITCH_PIN_1) ? 1 : 0;
    
    // Combine the two bits to get the gear position
    uint8_t gearPosition = (gearBit1 << 1) | gearBit0;
    
    // Map the 2-bit value to gear positions
    switch(gearPosition) {
        case 0:
            return PARK;
        case 1:
            return DRIVE;
        case 2:
            return REVERSE;
        default:
            return PARK;  // Default to PARK for safety
    }
}

// Initialize all hardware pins for the car locking system
void CarLocking_Init(void)
{
    // Enable clocks for required ports
    SYSCTL->RCGCGPIO |= 0x23;    // Enable PortA (0x01), PortB (0x02), PortF (0x20)
    
    // Wait for clock to stabilize
    volatile uint32_t delay = SYSCTL->RCGCGPIO;
    
    // Unlock PF0 to allow configuration
    GPIOF->LOCK = 0x4C4F434B;    // Unlock GPIO Port F Commit Register
    GPIOF->CR |= UNLOCK_BUTTON_PIN; // Allow changes to PF0
    
    // Configure Port F (Buttons and Lock Status LED)
    GPIOF->DIR &= ~(LOCK_BUTTON_PIN | UNLOCK_BUTTON_PIN); // Inputs for manual buttons
    GPIOF->DIR |= LOCK_STATUS_PIN; // Output for Lock Status LED
    GPIOF->DEN |= (LOCK_BUTTON_PIN | UNLOCK_BUTTON_PIN | LOCK_STATUS_PIN);  // Enable digital
    GPIOF->PUR |= (LOCK_BUTTON_PIN | UNLOCK_BUTTON_PIN);  // Enable pull-up resistors
    
    // Configure Port A (Ignition and Driver Door Switch)
    GPIOA->DIR &= ~(IGNITION_SWITCH_PIN | DRIVER_DOOR_SWITCH_PIN); // Inputs
    GPIOA->DEN |= (IGNITION_SWITCH_PIN | DRIVER_DOOR_SWITCH_PIN);  // Enable digital
    GPIOA->PUR |= (IGNITION_SWITCH_PIN | DRIVER_DOOR_SWITCH_PIN);  // Enable pull-up resistors
    
    // Configure Port B (Gear Switch - now uses 2 pins for 3 states)
    GPIOB->DIR &= ~(GEAR_SWITCH_PINS); // Inputs for Gear Switch
    GPIOB->DEN |= (GEAR_SWITCH_PINS);  // Enable digital
    GPIOB->PUR |= (GEAR_SWITCH_PINS);  // Enable pull-up resistors
    
    // Initialize to unlocked state
    UnlockDoors();
}