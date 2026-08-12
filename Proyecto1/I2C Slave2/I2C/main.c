/*
 * NODO ESCLAVO 2  (nano)  -  direccion I2C 0x31
 */
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "I2C.h"
#include "UART.h"

#define SLAVE_ADDR  0x31

// ---- Registros ----
#define REG_TEMP    0
#define REG_RELAY   2
#define REG_SERVO360 4
#define REG_STEP_COND 5
#define REG_STEP_CHILE 6
#define REG_STATUS  7

// ---- Pines analogicos ----
#define HW503_CH   1   // A1

// ---- Actuadores en PORTB ----
#define RELAY_BIT  PB3   // D11
// Servo360 -> OC1B = PB2 = D10

// ---- Steppers ----
// 28BYJ-48 (media paso): ~4096 pasos por vuelta del eje de salida. Ajustable.
#define PASOS_POR_VUELTA  4096
// Secuencia de media-paso, bits: b0=IN1 b1=IN2 b2=IN3 b3=IN4
static const uint8_t SEQ[8] = { 0b0001,0b0011,0b0010,0b0110,0b0100,0b1100,0b1000,0b1001 };

volatile uint16_t pasos_cond  = 0, pasos_chile = 0;
volatile uint8_t  idx_cond    = 0, idx_chile   = 0;

// Escribe las 4 bobinas del stepper CONDIMENTO (PD2..PD5)
static inline void bobinas_cond(uint8_t p){
    PORTD = (PORTD & ~(0x0F << 2)) | ((p & 0x0F) << 2);
}
// Escribe las 4 bobinas del stepper CHILE (IN1=PD6,IN2=PD7,IN3=PB0,IN4=PB1)
static inline void bobinas_chile(uint8_t p){
    PORTD = (PORTD & ~(0x03 << 6)) | ((p & 0x03) << 6);          // IN1,IN2
    PORTB = (PORTB & ~0x03)        | ((p >> 2) & 0x03);          // IN3,IN4
}

// ---- Servo360 (Timer1 Fast PWM, OC1B/PB2). Neutro=stop; <neutro y >neutro giran en sentidos opuestos. ----
#define S360_STOP  3000   // ~1.5 ms (parado)
#define S360_MEDIA 2600   // ~1.3 ms (media velocidad, sentido CONTRARIO al anterior)
static void servo360_init(void){
    DDRB |= (1<<PB2);
    TCCR1A = (1<<COM1B1)|(1<<WGM11);
    TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS11);   // Fast PWM modo 14, prescaler 8
    ICR1   = 39999;                              // 20 ms
    OCR1B  = S360_STOP;
}

// ---- Timer2: reloj de paso de los steppers (NO bloqueante) ----
static void steppers_init(void){
    // Salidas de bobinas
    DDRD |= (1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7);
    DDRB |= (1<<PB0)|(1<<PB1);
    bobinas_cond(0); bobinas_chile(0);
    // CTC, prescaler 1024 -> 15625 Hz; OCR2A=23 -> ~1.5 ms por paso (~650 pasos/s)
    TCCR2A = (1<<WGM21);
    TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);
    OCR2A  = 23;
    TIMSK2 = (1<<OCIE2A);
}

ISR(TIMER2_COMPA_vect){
    if (pasos_cond){
        idx_cond = (idx_cond + 1) & 7;
        bobinas_cond(SEQ[idx_cond]);
        if (--pasos_cond == 0) bobinas_cond(0);   // libera bobinas al terminar
    }
    if (pasos_chile){
        idx_chile = (idx_chile + 1) & 7;
        bobinas_chile(SEQ[idx_chile]);
        if (--pasos_chile == 0) bobinas_chile(0);
    }
}

// ---- ADC ----
static void initADC(void){
    ADMUX  = (1<<REFS0)|(1<<ADLAR);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
static uint8_t leerADC(uint8_t canal){
    ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
    ADCSRA |= (1<<ADSC);
    while (ADCSRA & (1<<ADSC));
    return ADCH;
}

int main(void){
    DDRB |= (1<<RELAY_BIT);
    initADC();
    servo360_init();
    steppers_init();
    initUART9600();
    I2C_Slave_Init(SLAVE_ADDR);
    sei();

    uint16_t t = 0;
    while (1){
        // ----- Sensores -> registros -----
        I2C_Slave_Buffer[REG_TEMP] = leerADC(HW503_CH);

        // ----- Comandos simples -----
        if (I2C_Slave_Buffer[REG_RELAY]) PORTB |= (1<<RELAY_BIT); else PORTB &= ~(1<<RELAY_BIT);
        OCR1B = (I2C_Slave_Buffer[REG_SERVO360]) ? S360_MEDIA : S360_STOP;

        // ----- Steppers: el maestro pide N vueltas; lo consumimos al aceptar -----
        uint8_t busy0, busy1;
        cli(); busy0 = (pasos_cond != 0); busy1 = (pasos_chile != 0); sei();

        if (I2C_Slave_Buffer[REG_STEP_COND] && !busy0){
            uint16_t n = (uint16_t)I2C_Slave_Buffer[REG_STEP_COND] * PASOS_POR_VUELTA;
            cli(); pasos_cond = n; sei();
            I2C_Slave_Buffer[REG_STEP_COND] = 0;      // consumido
        }
        if (I2C_Slave_Buffer[REG_STEP_CHILE] && !busy1){
            uint16_t n = (uint16_t)I2C_Slave_Buffer[REG_STEP_CHILE] * PASOS_POR_VUELTA;
            cli(); pasos_chile = n; sei();
            I2C_Slave_Buffer[REG_STEP_CHILE] = 0;
        }

        // ----- Estado (busy) para el maestro -----
        cli(); busy0 = (pasos_cond != 0); busy1 = (pasos_chile != 0); sei();
        I2C_Slave_Buffer[REG_STATUS] = (busy0 ? 1 : 0) | (busy1 ? 2 : 0);

        // ----- Diagnostico UART cada ~500 ms -----
        if (++t >= 25){
            t = 0;
            writeTextUART("Temp:");  writeNumberUART(I2C_Slave_Buffer[REG_TEMP]);
            writeTextUART(" Busy:"); writeNumberUART(I2C_Slave_Buffer[REG_STATUS]);
            writeTextUART("\r\n");
        }
        _delay_ms(20);
    }
}
