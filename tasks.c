#include "system_config.h"
#include "door_system.h"
#include "speed_system.h"
#include "parking_system.h"
#include "lcd.h"

// Door Task - Handles door locking/unlocking logic
void vDoorTask(void *pvParameters) {
    while(1) {
        DoorSystem_Update();
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay
    }
}

// Speed Task - Monitors vehicle speed and controls auto-lock
void vSpeedTask(void *pvParameters) {
    while(1) {
        SpeedSystem_Update();
        vTaskDelay(pdMS_TO_TICKS(200)); // 200ms delay
    }
}

// Parking Task - Handles ultrasonic sensor and parking assistance
void vParkingTask(void *pvParameters) {
    while(1) {
        ParkingSystem_Update();
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay
    }
}

// Display Task - Updates LCD with system status
void vDisplayTask(void *pvParameters) {
    char displayMsg[32];
    
    while(1) {
        if(xQueueReceive(xDisplayQueue, displayMsg, pdMS_TO_TICKS(100)) == pdTRUE) {
            xSemaphoreTake(xLCDMutex, portMAX_DELAY);
            LCD_set_cursor(0, 0);
            LCD_write_string(displayMsg);
            xSemaphoreGive(xLCDMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms delay
    }
} 