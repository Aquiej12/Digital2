#define F_CPU 16000000UL // Define CPU frequency as 16 MHz
#include <avr/io.h> 
#include <avr/delay.h>

#include "LCD/LCD.h" 

int main(void) {
    initLCD4bits();
    LCD_Set_Cursor(1,1);
    LCD_Write_Char('H');
    LCD_Write_Char('o');
    LCD_Write_Char('l');   
    LCD_Write_Char('a');
    LCD_Set_Cursor(2,1);
    LCD_Write_String("Mundo!");

    while (1)
    {
        /* code */
    }
    
}