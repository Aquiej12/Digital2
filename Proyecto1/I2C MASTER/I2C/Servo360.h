/*
 * Servo360.h  -  Servo de giro continuo en D13 (PB5), PWM por software
 * (Timer1 en modo normal + toggle manual del pin; D13 no tiene salida
 * de comparador de hardware, por eso no se puede usar Fast PWM aqui).
 */
#ifndef SERVO360_H_
#define SERVO360_H_

#include <stdint.h>

void Servo360_Init(void);

// Dispara un giro de 2 segundos a 50% de velocidad (no bloqueante).
// Si ya esta girando, no hace nada (no reinicia el conteo).
void Servo360_Girar(void);

#endif /* SERVO360_H_ */
