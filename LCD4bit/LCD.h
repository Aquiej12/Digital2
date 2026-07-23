#ifndef LCD_H_
#define LCD_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/delay.h>
#define RS (1<<PORTC0)
#define E (1<<PORTC1)

// Funcion para inicializar el LCD en modo 4 bits
void initLCD4bits(void);
// Funcion para colocar en el puerto un valor
void LCD_Port(char a);
// Funcion para enviar un comando
void LCD_Cmd(char a);
// Funcion para enviar un caracter
void LCD_Write_Char(char c);
// Funcion para enviar una cadena
void LCD_Write_String(char *a);
// Dezplazamiento a la derecha
void LCD_Shift_Right(void);
// Dezplazamiento a la izquierda
void LCD_Shift_Left(void);
// Establecer el cursor
void LCD_Set_Cursor(char c, char f);
#endif /* LCD_H_ */
