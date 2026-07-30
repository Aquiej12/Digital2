/*
 * UART.c
 *
 * Created: 23/07/2026
 *  Author: abner
 */

#include "UART.h"

void initUART9600(void)
{
    // 9600 baud @ 16 MHz -> UBRR = 103
    UBRR0H = 0;
    UBRR0L = 103;
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);   // Habilitar RX y TX
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8 bits de datos, 1 stop, sin paridad
}

void writeUART(char caracter)
{
    while(!(UCSR0A & (1<<UDRE0))); // Esperar a que el buffer este vacio
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
