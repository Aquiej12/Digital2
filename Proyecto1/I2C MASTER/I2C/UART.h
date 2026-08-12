/*
 * UART.h
 */
#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdint.h>

void initUART9600(void);
void writeUART(char caracter);
void writeTextUART(char* texto);
void writeNumberUART(uint16_t numero);

// Lectura NO bloqueante: devuelve el byte recibido, o -1 si no hay nada.
int16_t readUART(void);

#endif /* UART_H_ */
