#include "lcd.h"

int main(void) {
    // Initialize LCD (and I2C inside LCD_Init)
    LCD_Init();

    // Clear LCD and set cursor
    LCD_Clear();
    LCD_set_cursor(0, 0);

    // Write a test message
    LCD_write_string("Hello TM4C123!");
	
		LCD_set_cursor(1, 0);
	
		LCD_print_int(12345);

    // Loop forever
    while (1) {
        // Optionally, you can toggle something or just idle
    }
}
