/*
 * Laboratorio3-SPI.c   (ESCLAVO)
 *
 * Autor: Abner Quiej y Edwin Parada
 */

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "SPI.h"


volatile uint8_t valorLEDs = 0; 
volatile uint8_t pot1 = 0;
volatile uint8_t pot2 = 0;
volatile uint8_t flag_ignorar_dummy = 0; 

void mostrarEnLeds(uint8_t valor);
void initADC(void);
unsigned int ADC_Read(char canal);

int main(void)
{
    spiInit(SPI_SLAVE_SS, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
    initADC();
    SPCR |= (1<<SPIE); 
    sei(); 
	
    // Deshabilitar RX y TX 
    UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0));
    // Puerto D como salidas
    DDRD = 0xFF; 
    mostrarEnLeds(0);
    
    while (1)
    {
        // Leer ADC 
        pot1 = ADC_Read(0) >> 2; 
        pot2 = ADC_Read(1) >> 2; 
        
        _delay_ms(100);
        
        // Actualizar los LEDs
        mostrarEnLeds(valorLEDs);
    }
}


void mostrarEnLeds(uint8_t valor)
{
    PORTD = valor;
}

void initADC(void)
{

    ADMUX  = (1 << REFS0);
    // Habilitar ADC, Prescaler de 128
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

unsigned int ADC_Read(char canal)
{
    // Seleccionar el canal ADC
    ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
    _delay_us(10);
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));

    return ADC;
}

// INTERRUPCIÓN SPI 

ISR(SPI_STC_vect)
{
    uint8_t spiValor = SPDR; 
    
    // Si es dummy
    if (flag_ignorar_dummy == 1) {
        flag_ignorar_dummy = 0; 
        return; /
    }
    
    // Si no es dummy
    if(spiValor == 'c'){
        SPDR = pot1;            
        flag_ignorar_dummy = 1; 
    }
    else if(spiValor == 'd'){
        SPDR = pot2;            
        flag_ignorar_dummy = 1; 
    }
    else {
        valorLEDs = spiValor;
    }
}