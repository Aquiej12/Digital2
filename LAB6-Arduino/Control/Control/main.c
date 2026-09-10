#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "UART.h"

#define BTN_ARRIBA    PD2
#define BTN_ABAJO     PD3
#define BTN_IZQUIERDA PD4
#define BTN_DERECHA   PD5
#define BTN_A         PD6
#define BTN_B         PD7

#define LED_DEBUG     PC1

#define DEBOUNCE_MS   250

volatile uint8_t flagArriba = 0, flagAbajo = 0, flagIzquierda = 0;
volatile uint8_t flagDerecha = 0, flagA = 0, flagB = 0;
volatile uint8_t pinDAnterior = 0xFF;

void init_bt(void);

int main(void) {
	init_bt();
	initUART9600();
	sei();

	writeTextUART("Hola, funciona el UART");

	while (1) {
if (flagArriba) {
	flagArriba = 0;
	PORTC |= (1 << LED_DEBUG);
	writeUART('U');          // un solo carácter, no string
	_delay_ms(DEBOUNCE_MS);
	PORTC &= ~(1 << LED_DEBUG);
}
if (flagAbajo) {
	flagAbajo = 0;
	PORTC |= (1 << LED_DEBUG);
	writeUART('D');
	_delay_ms(DEBOUNCE_MS);
	PORTC &= ~(1 << LED_DEBUG);
}
if (flagIzquierda) {
	flagIzquierda = 0;
	PORTC |= (1 << LED_DEBUG);
	writeUART('L');
	_delay_ms(DEBOUNCE_MS);
	PORTC &= ~(1 << LED_DEBUG);
}
if (flagDerecha) {
	flagDerecha = 0;
	PORTC |= (1 << LED_DEBUG);
	writeUART('R');
	_delay_ms(DEBOUNCE_MS);
	PORTC &= ~(1 << LED_DEBUG);
}
if (flagA) {
	flagA = 0;
	PORTC |= (1 << LED_DEBUG);
	writeUART('A');
	_delay_ms(DEBOUNCE_MS);
	PORTC &= ~(1 << LED_DEBUG);
}
if (flagB) {
	flagB = 0;
	PORTC |= (1 << LED_DEBUG);
	writeUART('B');
	_delay_ms(DEBOUNCE_MS);
	PORTC &= ~(1 << LED_DEBUG);
}
	}
}

void init_bt(void) {
	// LED para indicar que se presiono un botont
	DDRC |= (1 << LED_DEBUG);
	PORTC &= ~(1 << LED_DEBUG);

	// Botones PD2-PD7 como entradas con pull-up
	DDRD &= ~((1<<BTN_ARRIBA)|(1<<BTN_ABAJO)|(1<<BTN_IZQUIERDA)|(1<<BTN_DERECHA)|(1<<BTN_A)|(1<<BTN_B));
	PORTD |= (1<<BTN_ARRIBA)|(1<<BTN_ABAJO)|(1<<BTN_IZQUIERDA)|(1<<BTN_DERECHA)|(1<<BTN_A)|(1<<BTN_B);

	pinDAnterior = PIND;

	// Pin Change Interrupt en PORTD
	PCICR  |= (1 << PCIE2);
	PCMSK2 |= (1<<PCINT18)|(1<<PCINT19)|(1<<PCINT20)|(1<<PCINT21)|(1<<PCINT22)|(1<<PCINT23);
}

ISR(PCINT2_vect) {
	uint8_t pinDActual = PIND;
	uint8_t cambios = pinDAnterior ^ pinDActual;
	pinDAnterior = pinDActual;

	uint8_t presionados = cambios & ~pinDActual;

	if (presionados & (1 << BTN_ARRIBA))    flagArriba    = 1;
	if (presionados & (1 << BTN_ABAJO))     flagAbajo     = 1;
	if (presionados & (1 << BTN_IZQUIERDA)) flagIzquierda = 1;
	if (presionados & (1 << BTN_DERECHA))   flagDerecha   = 1;
	if (presionados & (1 << BTN_A))         flagA         = 1;
	if (presionados & (1 << BTN_B))         flagB         = 1;
}