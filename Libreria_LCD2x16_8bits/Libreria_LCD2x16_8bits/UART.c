/*
 * UART.c
 *
 * Created: 16/07/2026
 *  Author: abner
 */

#include "UART.h"

// Funcion para inicializar el UART (con interrupcion de recepcion)
void initUART(void)
{
	UBRR0H = (uint8_t)(UBRR_VAL >> 8);
	UBRR0L = (uint8_t)(UBRR_VAL);

	DDRD  &= ~(1 << DDD0);					// RX entrada
	DDRD  |=  (1 << DDD1);					// TX salida

	UCSR0A = 0;
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);			// 8 bits de dato
}

// Funcion para enviar un caracter
void writeChar(char caracter)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = caracter;
}

// Funcion para enviar una cadena
void writeString(char* string)
{
	for (uint8_t i = 0; string[i] != '\0'; i++)
		writeChar(string[i]);
}

// Funcion para enviar un numero entero
void writeInt(int numero)
{
	char buffer[7];
	int i = 0;
	unsigned int magnitud;

	if (numero == 0) {
		writeChar('0');
		return;
	}

	if (numero < 0) {
		writeChar('-');
		magnitud = -numero;
	}
	else
		magnitud = numero;

	while (magnitud > 0) {					// los digitos salen al reves
		buffer[i] = '0' + (magnitud % 10);
		magnitud /= 10;
		i++;
	}

	while (i > 0) {						// y aqui se imprimen en orden
		i--;
		writeChar(buffer[i]);
	}
}
