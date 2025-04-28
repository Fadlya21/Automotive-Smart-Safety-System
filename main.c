#include "FreeRTOS.h"
#include "task.h"
#include "tm4c123gh6pm.h"

float timeAfterDelayExample;
float timeAfterDelayUntilExample;
float timeBeforeDelayUntilExample;
float timeBeforeDelayExample;
float TotalDelayUntilExample;
float TotalDelayExample;


void doSomething(){
	int x=0;
	while(x<100000){
		x++;
	}
}


void Toggle_LED1(void) {
}

void Toggle_LED2(void) {
}

float Get_System_Time_Seconds(void) {
    return (float)(xTaskGetTickCount()) / configTICK_RATE_HZ;
}

void vTaskDelayUntilExample(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(500);

    xLastWakeTime = xTaskGetTickCount(); 

    for (;;) {
        timeBeforeDelayUntilExample = Get_System_Time_Seconds(); 
        doSomething();
			  Toggle_LED2();                                                


        vTaskDelayUntil(&xLastWakeTime, xFrequency);  // 500ms delay

        timeAfterDelayUntilExample = Get_System_Time_Seconds(); 
			
				TotalDelayUntilExample=timeAfterDelayUntilExample-timeBeforeDelayUntilExample;
    }
}



void vTaskDelayExample(void *pvParameters) {
    for (;;) {
        timeBeforeDelayExample = Get_System_Time_Seconds();  
        Toggle_LED1();                                             
			
			  doSomething();
			
        vTaskDelay(pdMS_TO_TICKS(500));  // 500ms delay

         timeAfterDelayExample = Get_System_Time_Seconds();  
				TotalDelayExample=timeAfterDelayExample-timeBeforeDelayExample;
    }
}

int main(void) {
    xTaskCreate(vTaskDelayExample, "DelayTask", 100, NULL, 1, NULL);
    xTaskCreate(vTaskDelayUntilExample, "DelayUntilTask", 100, NULL, 1, NULL);

    vTaskStartScheduler();  

    while (1);  
}
