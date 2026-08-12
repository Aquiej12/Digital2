/*
 * I2C.h  -  Libreria I2C (TWI) para ATmega328P (Arduino Nano)
 * Maestro y Esclavo con protocolo de REGISTROS (lectura y escritura).
 *
 * Cada esclavo expone un buffer de registros I2C_Slave_Buffer[].
 *   - El maestro ESCRIBE:  [reg][valor]  -> guarda 'valor' en el registro 'reg'
 *   - El maestro LEE:      apunta 'reg' y luego lee N bytes consecutivos.
 * La aplicacion del esclavo llena los registros de sensores y lee los de comando.
 *
 * IMPORTANTE (esclavo): I2C_Slave_Init() ya define el ISR(TWI_vect).
 * Solo debes llamar sei() en tu main despues de inicializar.
 */
#ifndef I2C_H_
#define I2C_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdint.h>

// ---- Maestro (primitivas) ----
void    I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler);
uint8_t I2C_Master_Start(void);
uint8_t I2C_Master_RepeatedStart(void);
void    I2C_Master_Stop(void);
uint8_t I2C_Master_Write(uint8_t dato);
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack);

// ---- Maestro (alto nivel, protocolo de registros) ----
// Devuelven 1 si hubo ACK/exito, 0 si fallo (esclavo ausente, etc.)
uint8_t I2C_Master_WriteReg(uint8_t addr, uint8_t reg, uint8_t valor);
uint8_t I2C_Master_ReadReg (uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t n);

// ---- Esclavo ----
#define I2C_SLAVE_BUFFER_SIZE 8
extern volatile uint8_t I2C_Slave_Buffer[I2C_SLAVE_BUFFER_SIZE];
void    I2C_Slave_Init(uint8_t address);

#endif /* I2C_H_ */
