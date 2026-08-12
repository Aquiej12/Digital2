/*
 * ESCLAVO 1 - Direccion I2C 0x30
 
 */
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "I2C.h"

#define SLAVE_ADDR 0x30

#define IR_PIN              PD2  // D2
#define TRIG_PIN            PD3  // D3
#define ECHO_PIN            PD4  // D4
#define IN1_PIN             PD5  // D5 (L298 IN1)
#define IN2_PIN             PD6  // D6 (L298 IN2)
#define SERVO_CORREDERA_PIN PB3  // D11 (Servo 1)
#define SERVO_TAPADERA_PIN  PB2  // D10 (Servo 2)

#define REG_IR         0
#define REG_DIST       1
#define REG_SERVO_CORR 2
#define REG_SERVO_TAPA 3
#define REG_MOTOR      4

// PWM por software (Timer1, prescaler 8 -> 0.5us/tick)
#define TRAMA_TICKS  40000UL   // 20 ms (50 Hz)
volatile uint16_t pulso_corredera = 3000;  // 90 grados
volatile uint16_t pulso_tapadera  = 3000;  // 90 grados

// ---------- Motor DC (L298) ----------
static void motor_paro(void){       PORTD &= ~((1<<IN1_PIN)|(1<<IN2_PIN)); }
static void motor_avanzar(void){    PORTD |=  (1<<IN1_PIN); PORTD &= ~(1<<IN2_PIN); }
static void motor_retroceder(void){ PORTD &= ~(1<<IN1_PIN); PORTD |=  (1<<IN2_PIN); }

// ---------- Ultrasonico (Timer0) ----------
void init_Ultrasonico(void){
    TCCR0A = 0;
    TCCR0B = (1<<CS01)|(1<<CS00);   // prescaler 64 -> 1 tick = 4 us
}
uint16_t get_Distance(void){
    PORTD |= (1<<TRIG_PIN);
    _delay_us(10);
    PORTD &= ~(1<<TRIG_PIN);

    uint16_t timeout = 0;
    while (!(PIND & (1<<ECHO_PIN))){
        timeout++;
        _delay_us(1);
        if (timeout > 20000) return 999;
    }
    TCNT0 = 0;
    uint16_t ovf = 0;
    TIFR0 = (1<<TOV0);
    while (PIND & (1<<ECHO_PIN)){
        if (TIFR0 & (1<<TOV0)){ TIFR0 = (1<<TOV0); ovf++; }
        if (ovf > 200) return 999;
    }
    uint32_t time_us = (ovf * 256UL + TCNT0) * 4UL;
    return time_us / 58;
}

// ---------- Grados -> ticks ----------
uint16_t calcular_ticks(uint8_t grados){
    if (grados > 180) grados = 180;
    return 2000 + (((uint32_t)grados * 2000) / 180);  // 0=1ms(2000), 180=2ms(4000)
}

int main(void){
    // Entradas
    DDRD &= ~((1<<IR_PIN)|(1<<ECHO_PIN));
    // PORTD |= (1<<IR_PIN);   // pull-up del IR si tu modulo lo necesita
    // Salidas
    DDRD |= (1<<TRIG_PIN)|(1<<IN1_PIN)|(1<<IN2_PIN);
    DDRB |= (1<<SERVO_CORREDERA_PIN)|(1<<SERVO_TAPADERA_PIN);
    PORTD &= ~(1<<TRIG_PIN);
    motor_paro();

    init_Ultrasonico();

    // Timer1: PWM por software (modo normal, prescaler 8)
    TCCR1A = 0;
    TCCR1B = (1<<CS11);
    TIMSK1 = (1<<OCIE1A);
    OCR1A  = TCNT1 + 1000;

    I2C_Slave_Init(SLAVE_ADDR);
    sei();

    I2C_Slave_Buffer[REG_SERVO_CORR] = 90;
    I2C_Slave_Buffer[REG_SERVO_TAPA] = 90;
    I2C_Slave_Buffer[REG_MOTOR]      = 0;

    while (1){
        // ----- Sensores -> registros -----
        // IR: si tu modulo detecta con 0, invierte: (PIND & (1<<IR_PIN)) ? 0 : 1
        I2C_Slave_Buffer[REG_IR] = (PIND & (1<<IR_PIN)) ? 1 : 0;
        uint16_t d = get_Distance();
        I2C_Slave_Buffer[REG_DIST] = (d > 254) ? 255 : (uint8_t)d;  // 255 = sin eco/lejos

        // ----- Comandos -> servos (PWM software) -----
        uint16_t tc = calcular_ticks(I2C_Slave_Buffer[REG_SERVO_CORR]);
        uint16_t tt = calcular_ticks(I2C_Slave_Buffer[REG_SERVO_TAPA]);
        cli(); pulso_corredera = tc; pulso_tapadera = tt; sei();

        // ----- Comando -> motor DC -----
        switch (I2C_Slave_Buffer[REG_MOTOR]){
            case 1:  motor_avanzar();    break;
            case 2:  motor_retroceder(); break;
            default: motor_paro();       break;
        }

        _delay_ms(20);
    }
}

// PWM por software de los 2 servos (Timer1)
ISR(TIMER1_COMPA_vect){
    static uint8_t fase = 0;
    switch (fase){
        case 0:
            PORTB |= (1<<SERVO_CORREDERA_PIN);   // D11 alto
            OCR1A += pulso_corredera;
            fase = 1;
            break;
        case 1:
            PORTB &= ~(1<<SERVO_CORREDERA_PIN);  // D11 bajo
            PORTB |= (1<<SERVO_TAPADERA_PIN);    // D10 alto
            OCR1A += pulso_tapadera;
            fase = 2;
            break;
        default:
            PORTB &= ~(1<<SERVO_TAPADERA_PIN);   // D10 bajo
            OCR1A += (uint16_t)(TRAMA_TICKS - pulso_corredera - pulso_tapadera);
            fase = 0;
            break;
    }
}
