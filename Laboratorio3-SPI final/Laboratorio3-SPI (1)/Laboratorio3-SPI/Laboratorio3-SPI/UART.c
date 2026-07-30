/*
 * UART.c
 *
 * Created: 23/07/2026
 *  Author: Edwin Parada - Abner Quiej
 */

#include "UART.h"

void initUART9600(void)
{
    // 9600 baud
    UBRR0H = 0;
	//Bauds
    UBRR0L = 103;
	// Habilitar RX y TX
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	// 8 bits de datos
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

void writeUART(char caracter)
{
	//Esperar a que el buffer este vacio
    while(!(UCSR0A & (1<<UDRE0))); // 
    UDR0 = caracter;
}

void writeTextUART(char* texto)
{
    uint8_t i = 0;
    while(texto[i] != '\0'){
        writeUART(texto[i]);
        i++;
    }
}

void writeNumberUART(uint16_t numero)
{
    char buffer[6];
    uint8_t i = 0;

    if(numero == 0){
        writeUART('0');
        return;
    }
    while(numero > 0){
        buffer[i++] = '0' + (numero % 10);
        numero /= 10;
    }
    while(i > 0){
        writeUART(buffer[--i]);
    }
}


// Verificar si hay esperando un dato para leer
uint8_t availableUART(void)
{
	if (UCSR0A & (1<<RXC0)) return 1;
	else return 0;
}

// Lee y devolver el caracter recibido
char readUART(void)
{
	// Esperar a que el dato esté completamente recibido (evitar colisiones)
	while(!(UCSR0A & (1<<RXC0))); 
	return UDR0;
}