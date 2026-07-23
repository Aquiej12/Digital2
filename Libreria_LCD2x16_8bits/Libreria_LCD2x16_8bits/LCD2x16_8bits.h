/*
 * LCD2x16_8bits.h
 *
 * Created: 16/07/2026 16:19:41
 *  Author: abner
 */

#ifndef LCD2X16_8BITS_H_
#define LCD2X16_8BITS_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define RS (1<<PORTD2)
#define E  (1<<PORTD3)

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
