/*
 * MPR121.h  -  Libreria para el sensor tactil capacitivo MPR121 (NXP)
 * Se comunica por I2C usando I2C_Master_WriteReg/ReadReg (I2C.h).
 *
 * Modulo sin regulador ni level-shifter (alimentado a 3.3V, jumper ADD
 * cerrado a GND -> direccion 0x5A). Se lee por polling (sin usar INT).
 */
#ifndef MPR121_H_
#define MPR121_H_

#include <stdint.h>

#define MPR121_ADDR   0x5A

// Inicializa el chip: reset, filtros, umbrales touch/release y habilita los 12 electrodos.
void MPR121_Init(void);

// Mascara de 12 bits (bit N = electrodo N tocado). 0 si el modulo no responde.
uint16_t MPR121_LeerTouch(void);

// Calibracion persistente (EEPROM) del baseline de los electrodos 5-9 (nivel de agua).
// Guardar: captura el baseline actual del chip (usar con el recipiente vacio).
// Cargar:  si ya se guardo antes, lo aplica al chip; devuelve 0 si no habia nada guardado.
void    MPR121_GuardarBaseline(void);
uint8_t MPR121_CargarBaseline(void);

#endif /* MPR121_H_ */
