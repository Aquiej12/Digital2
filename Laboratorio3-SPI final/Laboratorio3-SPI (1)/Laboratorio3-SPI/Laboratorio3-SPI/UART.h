/*
 * UART.h
 *
 * Created: 23/07/2026
 *  Author: Edwin Parada - Abner Quiej
 */


#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdint.h>


uint8_t availableUART(void);
char readUART(void);
void initUART9600(void);
void writeUART(char caracter);
void writeTextUART(char* texto);
void writeNumberUART(uint16_t numero);

#endif /* UART_H_ */
