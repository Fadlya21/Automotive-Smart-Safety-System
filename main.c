#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "car_locking.h"
#include "adc_speed.h"
#include "ultrasonic.h"
#include "lcd_display.h"
#include "alerts.h"

// Task handles
static TaskHandle_t xSpeedMonitorTask;
static TaskHandle_t xDoorControlTask;
static TaskHandle_t xDistanceMonitorTask;
static TaskHandle_t xDisplayTask;

// Queue handles
static QueueHandle_t xSpeedQueue;
static QueueHandle_t xDistanceQueue;

// Mutex for shared resources
static SemaphoreHandle_t xDisplayMutex;
static SemaphoreHandle_t xAlertsMutex;

// Function prototypes for tasks
static void vSpeedMonitorTask(void *pvParameters);
static void vDoorControlTask(void *pvParameters);
static void vDistanceMonitorTask(void *pvParameters);
static void vDisplayTask(void *pvParameters);

int main(void)
{
    // Initialize all hardware components
    CarLocking_Init();
    SpeedSensor_Init();
    Ultrasonic_Init();
    LCD_Init();
    Alerts_Init();
    
    // Create queues for communication between tasks
    xSpeedQueue = xQueueCreate(1, sizeof(uint8_t));
    xDistanceQueue = xQueueCreate(1, sizeof(uint16_t));
    
    // Create mutex for shared resources
    xDisplayMutex = xSemaphoreCreateMutex();
    xAlertsMutex = xSemaphoreCreateMutex();
    
    // Create tasks
    xTaskCreate(vSpeedMonitorTask, "SpeedMonitor", configMINIMAL_STACK_SIZE, NULL, 2, &xSpeedMonitorTask);
    xTaskCreate(vDoorControlTask, "DoorControl", configMINIMAL_STACK_SIZE, NULL, 3, &xDoorControlTask);
    xTaskCreate(vDistanceMonitorTask, "DistanceMonitor", configMINIMAL_STACK_SIZE, NULL, 2, &xDistanceMonitorTask);
    xTaskCreate(vDisplayTask, "Display", configMINIMAL_STACK_SIZE, NULL, 1, &xDisplayTask);
    
    // Start the scheduler
    vTaskStartScheduler();
    
    while(1);
}

// Speed monitoring task - reads potentiometer and sends speed to queue
static void vSpeedMonitorTask(void *pvParameters)
{
    uint8_t speed;
    const TickType_t xDelay = pdMS_TO_TICKS(100); // 100ms
    
    for(;;)
    {
        // Read speed from potentiometer
        speed = ReadSpeed();
        
        // Send to queue for other tasks to use
        xQueueOverwrite(xSpeedQueue, &speed);
        
        vTaskDelay(xDelay);
    }
}

// Door control task - handles auto locking/unlocking based on speed, ignition, and manual inputs
static void vDoorControlTask(void *pvParameters)
{
    uint8_t speed;
    bool ignition;
    bool driverDoor;
    bool doorWarningActive = false;
    const TickType_t xDelay = pdMS_TO_TICKS(50); // 50ms
    
    for(;;)
    {
        // Get current speed from queue
        if(xQueuePeek(xSpeedQueue, &speed, 0) != pdTRUE)
        {
            speed = 0;
        }
        
        // Check manual inputs
        if(ManualLockPressed())
        {
            LockDoors();
            
            // Update display
            if(xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                LCD_DisplayDoorStatus(true);
                xSemaphoreGive(xDisplayMutex);
            }
            
            vTaskDelay(pdMS_TO_TICKS(200)); // Debounce
        }
        
        if(ManualUnlockPressed())
        {
            UnlockDoors();
            
            // Update display
            if(xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                LCD_DisplayDoorStatus(false);
                xSemaphoreGive(xDisplayMutex);
            }
            
            vTaskDelay(pdMS_TO_TICKS(200)); // Debounce
        }
        
        // Get current states
        ignition = ReadIgnitionSwitch();
        driverDoor = ReadDriverDoorSwitch();
        
        // Auto-lock logic - lock doors if speed exceeds 10 km/h
        if(speed > 10 && !GetLockStatus())
        {
            LockDoors();
            
            // Update display
            if(xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                LCD_DisplayDoorStatus(true);
                xSemaphoreGive(xDisplayMutex);
            }
        }
        
        // Auto-unlock when ignition is turned off
        if(ignition == OFF && GetLockStatus())
        {
            UnlockDoors();
            
            // Update display
            if(xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                LCD_DisplayDoorStatus(false);
                xSemaphoreGive(xDisplayMutex);
            }
        }
        
        // Door open warning when vehicle is moving
        bool shouldWarn = (speed > 0 && driverDoor == OPEN);
        
        if(shouldWarn != doorWarningActive)
        {
            doorWarningActive = shouldWarn;
            
            if(xSemaphoreTake(xAlertsMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                Alerts_DoorOpenWarning(doorWarningActive);
                xSemaphoreGive(xAlertsMutex);
            }
            
            if(doorWarningActive && xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                LCD_DisplayDoorOpenWarning();
                xSemaphoreGive(xDisplayMutex);
            }
        }
        
        vTaskDelay(xDelay);
    }
}

// Distance monitoring task - measures distance when in reverse and controls alerts
static void vDistanceMonitorTask(void *pvParameters)
{
    uint16_t distance;
    uint8_t gear;
    uint8_t zone;
    const TickType_t xDelay = pdMS_TO_TICKS(100); // 100ms
    
    for(;;)
    {
        gear = ReadGearSwitch();
        
        // Only measure distance when in reverse
        if(gear == REVERSE)
        {
            // Trigger distance measurement
            Ultrasonic_Trigger();
            
            // Get distance value
            distance = Ultrasonic_GetDistance();
            
            // Get zone based on distance
            zone = Ultrasonic_GetZone();
            
            // Update alerts
            if(xSemaphoreTake(xAlertsMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                Alerts_SetLED(zone);
                Alerts_SetProximityBuzzer(distance);
                xSemaphoreGive(xAlertsMutex);
            }
            
            // Send distance to queue for display
            xQueueOverwrite(xDistanceQueue, &distance);
        }
        else
        {
            // Not in reverse, turn off alerts
            if(xSemaphoreTake(xAlertsMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                Alerts_SetLED(ZONE_SAFE);
                Alerts_SetProximityBuzzer(0); // Disable
                xSemaphoreGive(xAlertsMutex);
            }
        }
        
        vTaskDelay(xDelay);
    }
}

// Display task - updates LCD with system status
static void vDisplayTask(void *pvParameters)
{
    uint8_t speed;
    uint16_t distance;
    uint8_t gear;
    const TickType_t xDelay = pdMS_TO_TICKS(200); // 200ms
    
    for(;;)
    {
        if(xSemaphoreTake(xDisplayMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            // Get current speed
            if(xQueuePeek(xSpeedQueue, &speed, 0) == pdTRUE)
            {
                LCD_DisplaySpeed(speed);
            }
            
            // Get gear state
            gear = ReadGearSwitch();
            
            // If in reverse, display distance
            if(gear == REVERSE)
            {
                if(xQueuePeek(xDistanceQueue, &distance, 0) == pdTRUE)
                {
                    LCD_DisplayDistance(distance);
                }
            }
            
            xSemaphoreGive(xDisplayMutex);
        }
        
        vTaskDelay(xDelay);
    }
}