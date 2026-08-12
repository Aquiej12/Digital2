/*
 * MPR121.c  -  Libreria para el sensor tactil capacitivo MPR121 (NXP)
 */
#include "MPR121.h"
#include "I2C.h"
#include <util/delay.h>
#include <avr/eeprom.h>

// ---- Registros NXP ----
#define MPR121_TOUCHSTATUS_L 0x00
#define MPR121_TOUCHSTATUS_H 0x01
#define MPR121_MHDR   0x2B
#define MPR121_NHDR   0x2C
#define MPR121_NCLR   0x2D
#define MPR121_FDLR   0x2E
#define MPR121_MHDF   0x2F
#define MPR121_NHDF   0x30
#define MPR121_NCLF   0x31
#define MPR121_FDLF   0x32
#define MPR121_NHDT   0x33
#define MPR121_NCLT   0x34
#define MPR121_FDLT   0x35
#define MPR121_TOUCHTH_0   0x41   // pares touch/release: 0x41/0x42 .. 0x57/0x58 (12 electrodos)
#define MPR121_RELEASETH_0 0x42
#define MPR121_DEBOUNCE 0x5B
#define MPR121_CONFIG1  0x5C
#define MPR121_CONFIG2  0x5D
#define MPR121_ECR      0x5E
#define MPR121_SOFTRESET 0x80
#define MPR121_BASELINE_0 0x1E   // ELE0_BASELINE .. ELE12_BASELINE (0x1E-0x2A)

#define MPR121_TOUCH_THRESHOLD   2    // bajado de 4 -> recipiente mas grueso de lo previsto
#define MPR121_RELEASE_THRESHOLD 1    // bajado de 2

// ---- Calibracion de baseline en EEPROM (electrodos 6-9, los del nivel de agua) ----
// Se perdio el electrodo 5 (fisicamente), ahora son 4 puntos: 6,7,8,9.
#define MPR121_ELE_MIN 6
#define MPR121_ELE_MAX 9
#define EEPROM_MARCA_ADDR    ((uint8_t*)0)
#define EEPROM_BASELINE_ADDR ((uint8_t*)1)
#define EEPROM_MARCA_VALOR   0xB4   // distinto del valor anterior (0xA5): invalida
                                    // cualquier calibracion vieja guardada con 5 electrodos

void MPR121_Init(void){
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_SOFTRESET, 0x63);
    _delay_ms(1);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_ECR, 0x00);   // detener mientras se configura

    I2C_Master_WriteReg(MPR121_ADDR, MPR121_MHDR, 0x01);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_NHDR, 0x01);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_NCLR, 0x0E);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_FDLR, 0x00);

    I2C_Master_WriteReg(MPR121_ADDR, MPR121_MHDF, 0x01);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_NHDF, 0x05);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_NCLF, 0x01);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_FDLF, 0x00);

    I2C_Master_WriteReg(MPR121_ADDR, MPR121_NHDT, 0x00);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_NCLT, 0x00);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_FDLT, 0x00);

    I2C_Master_WriteReg(MPR121_ADDR, MPR121_DEBOUNCE, 0x00);
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_CONFIG1, 0x3F);   // corriente de carga al maximo (63uA), mas sensible
    I2C_Master_WriteReg(MPR121_ADDR, MPR121_CONFIG2, 0x60);   // tiempo de carga 4us (antes 1us), mas sensible

    for (uint8_t i = 0; i < 12; i++){
        I2C_Master_WriteReg(MPR121_ADDR, MPR121_TOUCHTH_0   + 2*i, MPR121_TOUCH_THRESHOLD);
        I2C_Master_WriteReg(MPR121_ADDR, MPR121_RELEASETH_0 + 2*i, MPR121_RELEASE_THRESHOLD);
    }

    I2C_Master_WriteReg(MPR121_ADDR, MPR121_ECR, 0x8F);   // 12 electrodos, modo corriendo
}

uint16_t MPR121_LeerTouch(void){
    uint8_t buf[2];
    if (!I2C_Master_ReadReg(MPR121_ADDR, MPR121_TOUCHSTATUS_L, buf, 2)) return 0;
    return (((uint16_t)buf[1] << 8) | buf[0]) & 0x0FFF;
}

// Lee el baseline actual de los electrodos 5-9 del chip y lo guarda en EEPROM.
// Llamar con el recipiente en el estado que se quiere usar como referencia
// (por ahora: vacio), despues de dejar que el baseline se asiente unos ms.
void MPR121_GuardarBaseline(void){
    for (uint8_t e = MPR121_ELE_MIN; e <= MPR121_ELE_MAX; e++){
        uint8_t val = 0;
        I2C_Master_ReadReg(MPR121_ADDR, MPR121_BASELINE_0 + e, &val, 1);
        eeprom_update_byte(EEPROM_BASELINE_ADDR + (e - MPR121_ELE_MIN), val);
    }
    eeprom_update_byte(EEPROM_MARCA_ADDR, EEPROM_MARCA_VALOR);
}

// Si hay un baseline guardado en EEPROM, lo escribe en el chip (electrodos 5-9)
// para que arranque desde esa referencia en vez de recalibrar con lo que haya
// en ese momento (por ejemplo, el recipiente ya lleno). Devuelve 1 si aplico
// un baseline guardado, 0 si no habia nada guardado todavia.
uint8_t MPR121_CargarBaseline(void){
    if (eeprom_read_byte(EEPROM_MARCA_ADDR) != EEPROM_MARCA_VALOR) return 0;

    for (uint8_t e = MPR121_ELE_MIN; e <= MPR121_ELE_MAX; e++){
        uint8_t val = eeprom_read_byte(EEPROM_BASELINE_ADDR + (e - MPR121_ELE_MIN));
        I2C_Master_WriteReg(MPR121_ADDR, MPR121_BASELINE_0 + e, val);
    }
    return 1;
}
