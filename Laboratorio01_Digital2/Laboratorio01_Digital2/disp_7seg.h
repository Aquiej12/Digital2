/*
 * disp_7seg.h
 *
 * Libreria para manejar un display de 7 segmentos con los 8 pines
 * (A, B, C, D, E, F, G, DP) repartidos en cualquier puerto/pin del
 * ATmega328P. Cada segmento se configura por separado en disp7seg_init,
 * asi que se pueden mezclar puertos libremente (por ejemplo, dejar PORTD
 * libre para otra cosa y usar PORTB/PORTC para el display).
 *
 * Display de tipo ANODO COMUN: un segmento se enciende poniendo su pin en 0.
 */

#ifndef DISP_7SEG_H
#define DISP_7SEG_H

#include <stdint.h>
#include <avr/io.h>

// Configura los 8 segmentos. Para cada uno se indica el registro PORT
void disp7seg_init(volatile uint8_t *portA, uint8_t pinA,
                    volatile uint8_t *portB, uint8_t pinB,
                    volatile uint8_t *portC, uint8_t pinC,
                    volatile uint8_t *portD, uint8_t pinD,
                    volatile uint8_t *portE, uint8_t pinE,
                    volatile uint8_t *portF, uint8_t pinF,
                    volatile uint8_t *portG, uint8_t pinG,
                    volatile uint8_t *portDP, uint8_t pinDP);

// Muestra un digito de 0 a 15 (0-9 y A-F) en el display. No toca el DP.
void displ_7seg(uint8_t num);

// Enciende o apaga el punto decimal por separado.
void disp7seg_dp(uint8_t encendido);

// Apaga todos los segmentos (incluido el DP).
void disp7seg_apagar(void);

#endif
