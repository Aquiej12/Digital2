/*
 * Libreria_LCD2x16_8bits.c
 *
 * RS -> PD2, E -> PD3, RW -> tierra
 * D0-D3 -> PD4-PD7, D4-D7 -> PB0-PB3
 * Pot 1 -> PC0, Pot 2 -> PC1
 * UART: TX -> PD1, RX -> PD0
 *
 * Created: 16/07/2026 16:11:45
 * Author : abner
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "LCD2x16_8bits.h"
#include "UART.h"

volatile int contador = 0;

// Funcion para inicializar el ADC
void ADC_Init(void){
	ADMUX  = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

// Funcion para leer un canal del ADC (0 = PC0, 1 = PC1)
unsigned int ADC_Read(char canal){
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
	_delay_us(10);

	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));

	return ADC;
}

int main(void){
	unsigned int pot1, pot2;

	cli();
	LCD16x2_8bits_init();
	ADC_Init();
	initUART();
	sei();

	// Titulos
	LCD16x2_8bits_set_cursor(1,1);
	LCD16x2_8bits_strg("S1:   S2:   S3:");

	while(1){
		pot1 = ADC_Read(0);
		pot2 = ADC_Read(1);

		// S1: pot 1 en voltaje
		LCD16x2_8bits_set_cursor(1,2);
		LCD16x2_8bits_float(pot1 * 5.0 / 1023.0, 2);
		LCD16x2_8bits_char('V');

		// S2: pot 2 en decimal (0 - 1023)
		LCD16x2_8bits_set_cursor(7,2);
		LCD16x2_8bits_int(pot2);
		LCD16x2_8bits_strg("    ");			// borra los digitos que sobran

		// S3: contador
		LCD16x2_8bits_set_cursor(13,2);
		LCD16x2_8bits_int(contador);
		LCD16x2_8bits_strg("   ");

		// Enviar las dos lecturas por UART
		writeString("Pot1: ");
		writeInt(pot1);
		writeString("  Pot2: ");
		writeInt(pot2);
		writeString("\r\n");

		_delay_ms(200);
	}
}

// Interrupcion de recepcion: + incrementa, - decrementa el contador
ISR(USART_RX_vect){
	char dato = UDR0;

	if(dato == '+')
		contador++;
	else if(dato == '-')
		contador--;
}
