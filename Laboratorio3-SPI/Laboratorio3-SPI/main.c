/*
 * Laboratorio3-SPI.c
 *
 * Created: 23/07/2026 15:37:30
 * Author : abner
 */

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "SPI.h"
#include "UART.h"


uint8_t pot1 = 0;
uint8_t pot2 = 0;


int main(void)
{
    DDRC |= (1<<DDC5);     // SS del esclavo como salida
    PORTC |= (1<<PORTC5);  // SS = 1 (inactivo)

    spiInit(SPI_MASTER_OSC_DIV4, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
    initUART9600();


    while (1)
    {
        PORTC &= ~(1<<PORTC5); // Seleccionar el Slave
        spiWrite('c');         // Enviar el comando de solicitud
        _delay_us(20);         // Tiempo para que el esclavo cargue pot1
        pot1 = spiRead();      // Leer el valor del ADC recibido por SPI
        PORTC |= (1<<PORTC5);  // Deseleccionar el Slave


        PORTC &= ~(1<<PORTC5); // Seleccionar el Slave
        spiWrite('d');         // Enviar el comando de solicitud
        _delay_us(20);         // Tiempo para que el esclavo cargue pot2
        pot2 = spiRead();      // Leer el valor del ADC recibido por SPI
        PORTC |= (1<<PORTC5);  // Deseleccionar el Slave

        // Enviar las lecturas a la terminal serial
        writeTextUART("Pot1: ");
        writeNumberUART(pot1);
        writeTextUART("  Pot2: ");
        writeNumberUART(pot2);
        writeTextUART("\r\n");

        _delay_ms(250);
    }
}
