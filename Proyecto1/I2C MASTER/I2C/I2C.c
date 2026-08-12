/*
 * I2C.c  -  Libreria I2C (TWI) para ATmega328P
 * Maestro + Esclavo con protocolo de registros.
 * Maestro con TIMEOUT: nunca se cuelga aunque el bus este mal.
 * LED de actividad I2C (esclavo): D13 (PB5) parpadea en cada transaccion.
 */
#include "I2C.h"
#include <avr/interrupt.h>

// ============================================================
//                        MAESTRO
// ============================================================
#define I2C_WAIT_MAX  30000    // ~11 ms; evita cuelgues por bus mal cableado

// Espera a TWINT con limite. Devuelve 1 = ok, 0 = timeout.
static uint8_t esperar_twint(void){
    uint16_t t = 0;
    while (!(TWCR & (1<<TWINT))){
        if (++t > I2C_WAIT_MAX) return 0;
    }
    return 1;
}

// Reinicia el modulo TWI si quedo en un estado colgado.
static void I2C_recover(void){
    TWCR = 0;
    TWCR = (1<<TWEN);
}

void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler){
    // SDA (PC4) y SCL (PC5) como entradas; el TWI toma el control.
    // Se necesitan pull-ups de 4.7k a 5V en SDA y SCL.
    DDRC &= ~((1<<DDC4)|(1<<DDC5));

    switch (Prescaler) {
        case 1:  TWSR &= ~((1<<TWPS1)|(1<<TWPS0)); break;
        case 4:  TWSR &= ~(1<<TWPS1); TWSR |= (1<<TWPS0); break;
        case 16: TWSR &= ~(1<<TWPS0); TWSR |= (1<<TWPS1); break;
        case 64: TWSR |= (1<<TWPS1)|(1<<TWPS0); break;
        default: TWSR &= ~((1<<TWPS1)|(1<<TWPS0)); Prescaler = 1; break;
    }
    TWBR = ((F_CPU/SCL_Clock)-16)/(2*Prescaler);
    TWCR = (1<<TWEN);
}

uint8_t I2C_Master_Start(void){
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if (!esperar_twint()) return 0;
    return ((TWSR & 0xF8) == 0x08);
}

uint8_t I2C_Master_RepeatedStart(void){
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if (!esperar_twint()) return 0;
    return ((TWSR & 0xF8) == 0x10);
}

void I2C_Master_Stop(void){
    TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);
    uint16_t t = 0;
    while ((TWCR & (1<<TWSTO)) && (++t < I2C_WAIT_MAX));
}

uint8_t I2C_Master_Write(uint8_t dato){
    TWDR = dato;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if (!esperar_twint()) return 0;     // 0 no es un status valido -> se trata como fallo
    return (TWSR & 0xF8);
}

uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack){
    if(ack) TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWEA);
    else    TWCR = (1<<TWEN)|(1<<TWINT);
    if (!esperar_twint()) return 0;
    *buffer = TWDR;
    return (TWSR & 0xF8);
}

// ---- Alto nivel: escribir un registro del esclavo ----
uint8_t I2C_Master_WriteReg(uint8_t addr, uint8_t reg, uint8_t valor){
    if(!I2C_Master_Start()){ I2C_recover(); return 0; }
    if((I2C_Master_Write((addr<<1)|0) & 0xF8) != 0x18){ I2C_Master_Stop(); return 0; } // SLA+W ACK?
    I2C_Master_Write(reg);
    I2C_Master_Write(valor);
    I2C_Master_Stop();
    return 1;
}

// ---- Alto nivel: leer 'n' bytes desde el registro 'reg' ----
uint8_t I2C_Master_ReadReg(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t n){
    if(!I2C_Master_Start()){ I2C_recover(); return 0; }
    if((I2C_Master_Write((addr<<1)|0) & 0xF8) != 0x18){ I2C_Master_Stop(); return 0; } // SLA+W
    I2C_Master_Write(reg);                 // fijar puntero de registro
    if(!I2C_Master_RepeatedStart()){ I2C_Master_Stop(); return 0; }
    if((I2C_Master_Write((addr<<1)|1) & 0xF8) != 0x40){ I2C_Master_Stop(); return 0; } // SLA+R ACK?
    for(uint8_t i=0;i<n;i++){
        I2C_Master_Read(&buffer[i], (i < (n-1)) ? 1 : 0); // ACK menos en el ultimo
    }
    I2C_Master_Stop();
    return 1;
}

// ============================================================
//                        ESCLAVO
// ============================================================
volatile uint8_t I2C_Slave_Buffer[I2C_SLAVE_BUFFER_SIZE];
static volatile uint8_t reg_ptr    = 0;
static volatile uint8_t first_byte = 1;

void I2C_Slave_Init(uint8_t address){
    DDRB |= (1<<PB5);         // D13 como salida (LED de actividad I2C)
    TWAR = (address << 1);    // direccion en bits 7..1
    // TWEN=habilita, TWEA=ACK, TWIE=interrupcion
    TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWIE);
}

ISR(TWI_vect){
    switch (TWSR & 0xF8){
        // ---- Recepcion (maestro escribe) ----
        case 0x60: // propia SLA+W recibida
        case 0x70: // llamada general
            first_byte = 1;
            PINB = (1<<PB5);            // parpadeo LED de actividad
            TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);
            break;
        case 0x80: // dato recibido, ACK
        case 0x90:
            if(first_byte){
                reg_ptr = TWDR % I2C_SLAVE_BUFFER_SIZE; // primer byte = registro
                first_byte = 0;
            } else {
                I2C_Slave_Buffer[reg_ptr] = TWDR;       // siguientes = valores
                reg_ptr = (reg_ptr + 1) % I2C_SLAVE_BUFFER_SIZE;
            }
            TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);
            break;

        // ---- Transmision (maestro lee) ----
        case 0xA8: // propia SLA+R recibida
        case 0xB0:
            PINB = (1<<PB5);            // parpadeo LED de actividad
            TWDR = I2C_Slave_Buffer[reg_ptr];
            reg_ptr = (reg_ptr + 1) % I2C_SLAVE_BUFFER_SIZE;
            TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);
            break;
        case 0xB8: // dato transmitido, ACK del maestro -> enviar mas
            TWDR = I2C_Slave_Buffer[reg_ptr];
            reg_ptr = (reg_ptr + 1) % I2C_SLAVE_BUFFER_SIZE;
            TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);
            break;

        // ---- Fin / limpieza ----
        case 0xC0: // dato transmitido, NACK del maestro (fin de lectura)
        case 0xC8:
        case 0xA0: // STOP o repeated start
        default:
            TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);
            break;
    }
}
