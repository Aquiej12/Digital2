/*
 * Laboratorio3-SPI.c   (ESCLAVO)
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

volatile uint8_t valorUART = 0;
volatile uint8_t pot1 = 0;
volatile uint8_t pot2 = 0;

void UARTout(void);
void initADC(void);
unsigned int ADC_Read(char canal);

int main(void)
{
    spiInit(SPI_SLAVE_SS, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
    initADC();
    SPCR |= (1<<SPIE); // Habilitar interrupciones SPI
    sei(); // Habilitar interrupciones globales
	// Iniciar portD como salida
	DDRD = 0xFF;
	// PORTD incialmente apagado
	PORTD = 0x00;
	//deshabilitar rx y tx
	UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0));

    while (1)
    {
        pot1 = ADC_Read(0) >> 2; // 8 bits altos del potenciometro en ADC0
        pot2 = ADC_Read(1) >> 2; // 8 bits altos del potenciometro en ADC1
        _delay_ms(100);
		UARTout();
    }
}

void UARTout(void)
{
	
	// Cargar valor de UART a PORTD
	PORTD = valorUART;
}

void initADC(void)
{
    ADMUX  = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    }

unsigned int ADC_Read(char canal){
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
	_delay_us(10);

	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));

	return ADC;
}


// El maestro envia 'c' para pedir pot1 y 'd' para pedir pot2, si no es ninguno de los dos:
// Guardar lo recibido en valorUART
ISR(SPI_STC_vect)
{
    uint8_t spiValor = SPDR;
    if(spiValor == 'c'){
        SPDR = pot1;
    }
    else if(spiValor == 'd'){
        SPDR = pot2;
    }
	
	valorUART = spiValor;
	
}
