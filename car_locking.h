#ifndef CAR_LOCKING_H
#define CAR_LOCKING_H

#include <stdint.h>
#include <stdbool.h>

// Initialization
/**
 * @brief Initializes all hardware pins for the car locking system
 * Must be called before using any other functions
 */
void CarLocking_Init(void);

// Control APIs
/**
 * @brief Locks all car doors
 */
void LockDoors(void);

/**
 * @brief Unlocks all car doors
 */
void UnlockDoors(void);

/**
 * @brief Get the current lock status
 * @return true if doors are locked, false otherwise
 */
bool GetLockStatus(void);

// Input APIs
/**
 * @brief Checks if manual lock button is pressed
 * @return true if pressed, false otherwise
 */
bool ManualLockPressed(void);

/**
 * @brief Checks if manual unlock button is pressed
 * @return true if pressed, false otherwise
 */
bool ManualUnlockPressed(void);

/**
 * @brief Reads the current ignition switch state
 * @return ON if ignition is on, OFF otherwise
 */
bool ReadIgnitionSwitch(void);

/**
 * @brief Reads the driver door switch state
 * @return OPEN if door is open, CLOSED otherwise
 */
bool ReadDriverDoorSwitch(void);

/**
 * @brief Reads the gear position switch
 * @return PARK, DRIVE, or REVERSE
 */
uint8_t ReadGearSwitch(void);

// Gear positions
#define PARK    0
#define DRIVE   1
#define REVERSE 2

// Ignition states
#define ON      true
#define OFF     false

// Driver door states
#define OPEN    true
#define CLOSED  false

#endif // CAR_LOCKING_H