/*
 * Laboratorio01_Digital2.c
 *
 * Created: 9/07/2026 17:28:22
 * Author: Abner Quiej
 * Description: Juego de carreras con ATmega328P (Arduino Nano). Dos jugadores
 *              compiten presionando un boton para avanzar hacia la meta,
 *              representada mediante un contador de decadas de 4 bits (LEDs).
 *              Antes de la carrera hay un conteo regresivo en display de 7 seg.
 */
/****************************************/
// Encabezado (Libraries)

// Velocidad del reloj del atmega328P
#define F_CPU 16000000UL      
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "disp_7seg.h"

/****************************************/
// Constantes de configuracion
#define TIMER1_OCR    249   // Interrupcion cada 1 ms
#define TIME_INIT     5     // segundos del conteo regresivo


// Estados de la FSM
#define ST_IDLE       0     // esperando boton de inicio
#define ST_COUNTDOWN  1     // conteo regresivo
#define ST_RACE       2     // carrera activa
#define ST_END        3     // Fin del Juego

/****************************************/
// Variables globales
volatile uint8_t counterp1 = 0;   // LEDs del Player 1
volatile uint8_t counterp2 = 0;   // LEDs del Player 2

volatile uint8_t flag_p1   = 0;   // Bandera: pulsacion de P1
volatile uint8_t flag_p2   = 0;   // Bandera: pulsacion de P2
volatile uint8_t flag_star = 0;   // Bandera: pulsacion de inicio

volatile uint8_t race_active = 0; // 1 = los botones de jugadores estan habilitados

volatile uint16_t ms_tick = 0;    // cuenta los ms que van pasando, de 0 a 999
volatile uint8_t  flag_1s  = 0;   // se pone en 1 cada vez que ms_tick llega a 1000

/****************************************/
// Function prototypes
void initPins(void);
void initTimer1(void);
void showLEDs(void);
void endGame(void);


/****************************************/
// Main Function
int main(void)
{
	// configura pines de entradas/salidas (LEDs y botones)
	initPins();      

	// Deshabilitar el UART (TX/RX), si no PD0 y PD1 no sirven como pines normales del display
	UCSR0B = 0x00;

	// inicializa display de 7 segmentos
	disp7seg_init(&PORTD, PD0, &PORTD, PD1, &PORTD, PD2, &PORTD, PD3,
	              &PORTD, PD4, &PORTD, PD5, &PORTD, PD6, &PORTD, PD7);

 	// configura el Timer1 para que interrumpa cada 1 ms
	initTimer1();    
	
	// habilita las interrupciones globales
	sei();

	uint8_t  state       = ST_IDLE;  // estado actual de la maquina de estados
	uint8_t  countdown    = 0;        // numero que va bajando (5,4,3,2,1,0) antes de correr


	while (1)
	{
		switch (state)
		{
			// --- Esperando el boton de inicio ---
			case ST_IDLE:
				if (flag_star)
				{
					flag_star  = 0;
					counterp1  = 0;
					counterp2  = 0;
					showLEDs();           // limpiar LEDs de la partida anterior
					countdown  = TIME_INIT;
					displ_7seg(countdown); // mostrar el 5 antes de empezar a bajar
					ms_tick    = 0;
					flag_1s    = 0;
					state = ST_COUNTDOWN;
				}
				break;

			// --- Conteo regresivo (botones de jugadores deshabilitados) ---
			case ST_COUNTDOWN:
				if (flag_1s)   // ya paso 1 segundo
				{
					flag_1s = 0;
					if (countdown == 0)
					{
						flag_p1 = 0;          // descartar cualquier pulsacion previa
						flag_p2 = 0;
						race_active = 1;      // habilitar botones de jugadores
						showLEDs();           // posicion inicial de los "autos"
						disp7seg_apagar();    // apagar el display, ya no hace falta
						state = ST_RACE;
					}
					else
					{
						countdown--;
						displ_7seg(countdown); // mostrar el nuevo numero
					}
				}
				break;

			// --- Carrera: rutinas independientes de cada jugador ---
			case ST_RACE:
				// Rutina Player 1: cuenta una vez por pulsacion 
				if (flag_p1)
				{
					flag_p1 = 0;
					counterp1++;
					showLEDs();
				}

				// Rutina Player 2: cuenta una vez por pulsacion
				if (flag_p2)
				{
					flag_p2 = 0;
					counterp2++;
					showLEDs();
				}

				// Revisar si alguien llego a la meta
				if (counterp1 >= 4 || counterp2 >= 4)
				{
					race_active = 0;   // deshabilitar botones de jugadores, ya no suman mas

					if (counterp1 >= 4)
					{
						// Gano el Jugador 1: sus 4 LEDs encendidos, los del otro apagados
						PORTB |= (1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3);
						PORTC &= ~((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3));
						displ_7seg(1);
					}
					else
					{
						// Gano el Jugador 2: sus 4 LEDs encendidos, los del otro apagados
						PORTC |= (1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);
						PORTB &= ~((1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3));
						displ_7seg(2);
					}

					state = ST_END;
				}
				break;

			// --- Fin del juego: permitir reinicio con el boton de inicio ---
			case ST_END:
				if (flag_star)
				{
					flag_star = 0;
					state = ST_IDLE;
				}
				break;
		}
	}
}

/****************************************/
// NON-Interrupt subroutines

// Configura el Timer1 en modo CTC (Clear Timer on Compare) para que
// genere una interrupcion cada 1 ms
void initTimer1(void)
{
	TCCR1A = 0;                                   
	TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10); 
	OCR1A  = TIMER1_OCR;                         
	TIMSK1 |= (1<<OCIE1A);                        
	TCNT1  = 0;                                   
}

// Iniciar los Pines que se utilizaran
void initPins(void)
{
	// --- PORTB: PB0-PB3 salidas (LEDs Player 1) ---
	DDRB  |=  (1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3);
	PORTB &= ~((1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3));

	// --- PORTB: PB4 entrada (boton Player 2) con pull-up ---
	DDRB  &= ~(1<<PB4);
	PORTB |=  (1<<PB4);

	// --- PORTC: PC0-PC3 salidas (LEDs Player 2) ---
	DDRC  |=  (1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);
	PORTC &= ~((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3));

	// --- PORTC: PC4 (inicio) y PC5 (Player 1) entradas con pull-up ---
	DDRC  &= ~((1<<PC4)|(1<<PC5));
	PORTC |=  (1<<PC4)|(1<<PC5);
}

// Prende, dentro de los 4 LEDs de cada jugador, solo el que corresponde
// a su posicion actual.
void showLEDs(void)
{
	
	PORTB = (PORTB & 0xF0) | ((counterp1 >= 1 && counterp1 <= 4) ? (1 << (counterp1 - 1)) : 0);
	PORTC = (PORTC & 0xF0) | ((counterp2 >= 1 && counterp2 <= 4) ? (1 << (counterp2 - 1)) : 0);
}


/****************************************/
// Interrupt routines

// Anti-rebote por muestreo cada 1 ms: se detecta el flanco de bajada

ISR(TIMER1_COMPA_vect)
{
	static uint8_t prev_p1   = 1;   // estado del pin de P1 en la lectura anterior
	static uint8_t prev_p2   = 1;   // estado del pin de P2 en la lectura anterior
	static uint8_t prev_star = 1;   // estado del pin de inicio en la lectura anterior

	// cuenta 1000 ms y avisa al main con una bandera
	ms_tick++;
	if (ms_tick >= 1000)
	{
		ms_tick = 0;
		flag_1s = 1;
	}

	// --- Boton de inicio (siempre habilitado, para iniciar/reiniciar) ---
	uint8_t curr_star = (PINC & (1<<PC4)) ? 1 : 0;
	if (prev_star == 1 && curr_star == 0) flag_star = 1;   
	prev_star = curr_star;

	// --- Botones de jugadores: solo durante la carrera ---
	if (race_active)
	{
		uint8_t curr_p1 = (PINC & (1<<PC5)) ? 1 : 0;
		uint8_t curr_p2 = (PINB & (1<<PB4)) ? 1 : 0;

		if (prev_p1 == 1 && curr_p1 == 0) flag_p1 = 1;   // Player 1
		if (prev_p2 == 1 && curr_p2 == 0) flag_p2 = 1;   // Player 2

		prev_p1 = curr_p1;
		prev_p2 = curr_p2;
	}
	else
	{
		prev_p1 = 1;   // dejar listo el flanco para la siguiente carrera
		prev_p2 = 1;
	}
}
