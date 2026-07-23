/*
 * UART.h
 *
 * Created: 16/07/2026
 *  Author: abner
 */

#ifndef UART_H_
#define UART_H_

#define F_CPU 16000000UL
#define BAUD  9600
#define UBRR_VAL (F_CPU/16/BAUD - 1)

#include <avr/io.h>

// Funcion para inicializar el UART (con interrupcion de recepcion)
void initUART(void);
// Funcion para enviar un caracter
void writeChar(char caracter);
// Funcion para enviar una cadena
void writeString(char* string);
// Funcion para enviar un numero entero
void writeInt(int numero);

#endif /* UART_H_ */
