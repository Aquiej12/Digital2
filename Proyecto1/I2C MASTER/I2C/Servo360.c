/*
 * Servo360.c  -  Servo de giro continuo en D13 (PB5), PWM por software.
 * Trama de 20 ms (ticks de 0.5us, prescaler 8 @16MHz): neutro=1500us(quieto),
 * 50% adelante a medio camino entre neutro y maximo (2000us).
 * La duracion (2s) se cuenta en tramas dentro del propio ISR, no depende
 * del loop principal -> no bloquea nada.
 */
#define F_CPU 16000000UL
#include "Servo360.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define SERVO_BIT PB5   // D13

#define TRAMA_TICKS    40000UL   // 20 ms
#define NEUTRO_TICKS   3000      // 1500us: quieto
#define VEL50_TICKS    3500      // 1750us: ~50% de velocidad hacia adelante

#define DURACION_TRAMAS 100      // 100 * 20ms = 2000ms = 2s

static volatile uint16_t pulso    = NEUTRO_TICKS;
static volatile uint8_t  girando  = 0;

void Servo360_Init(void){
    DDRB |= (1<<SERVO_BIT);
    TCCR1A = 0;
    TCCR1B = (1<<CS11);      // prescaler 8, modo normal (toggle manual del pin)
    TIMSK1 |= (1<<OCIE1A);
    OCR1A = TCNT1 + 1000;
}

void Servo360_Girar(void){
    if (!girando){
        pulso   = VEL50_TICKS;
        girando = 1;
    }
}

ISR(TIMER1_COMPA_vect){
    static uint8_t  fase = 0;
    static uint16_t contador_tramas = 0;

    if (fase == 0){
        PORTB |= (1<<SERVO_BIT);
        OCR1A += pulso;
        fase = 1;
    } else {
        PORTB &= ~(1<<SERVO_BIT);
        OCR1A += (uint16_t)(TRAMA_TICKS - pulso);
        fase = 0;

        if (girando){
            if (++contador_tramas >= DURACION_TRAMAS){
                pulso = NEUTRO_TICKS;
                girando = 0;
                contador_tramas = 0;
            }
        }
    }
}
