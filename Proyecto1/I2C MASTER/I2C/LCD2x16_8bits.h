/*
 * LCD2x16_8bits.h  -  Misma libreria del proyecto, pines adaptados al master.
 *
 * Conexion (8 bits, con RW en un pin propio en vez de a GND):
 *   RS - D2 (PD2)   RW - D3 (PD3)   E - D4 (PD4)
 *   D0 - D5 (PD5)   D1 - D6 (PD6)   D2 - D7 (PD7)   D3 - D8  (PB0)
 *   D4 - D9 (PB1)   D5 - D10 (PB2)  D6 - D11 (PB3)  D7 - D12 (PB4)
 */
#ifndef LCD2X16_8BITS_H_
#define LCD2X16_8BITS_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define RS (1<<PORTD2)
#define RW (1<<PORTD3)
#define E  (1<<PORTD4)

// Funcion para inicializar el LCD en modo 8 bits
void LCD16x2_8bits_init(void);
// Funcion para colocar en el puerto un valor
void LCD16x2_8bits_port(char a);
// Funcion para enviar un comando
void LCD16x2_8bits_cmd(char a);
// Funcion para enviar un caracter
void LCD16x2_8bits_char(char c);
// Funcion para enviar una cadena
void LCD16x2_8bits_strg(char *a);
// Funcion para enviar un numero entero
void LCD16x2_8bits_int(int numero);
// Funcion para enviar un numero con decimales
void LCD16x2_8bits_float(float numero, char decimales);
// Funcion para limpiar la pantalla
void LCD16x2_8bits_clear(void);
// Dezplazamiento a la derecha
void LCD16x2_8bits_shift_right(void);
// Dezplazamiento a la izquierda
void LCD16x2_8bits_shift_left(void);
// Establecer el cursor
void LCD16x2_8bits_set_cursor(char c, char f);

#endif /* LCD2X16_8BITS_H_ */
