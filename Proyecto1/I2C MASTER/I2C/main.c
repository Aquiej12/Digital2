/*
 * NODO MAESTRO  (nano 2) 
 */
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "I2C.h"
#include "MPR121.h"
#include "UART.h"
#include "LCD2x16_8bits.h"
#include "Servo360.h"

// ================= PARAMETROS AJUSTABLES =================
#define Distancia_1        35     // cm (corredera -> punto de condimentos)
#define Distancia_2        20     // cm (corredera -> punto de agua)
#define Distancia_3        8      // cm (corredera -> punto de tapa)
#define Segundos_agua      20     // s  (espera de llenado de agua, no bloqueante)
#define MIN3_MS            180000UL   // 3 min de coccion (Fase 10)
#define SERVO360_MS        5000UL     // 2 s de giro de la cafetera
#define SETTLE_MS          700UL      // asentar servos entre fases

#define VUELTAS_CONDIMENTO 3
#define VUELTAS_CHILE      2
#define DISPENSAR_CHILE    1      // 1 = tambien echa chile (luego se elige por tactil/Adafruit)

#define CORREDERA_AGARRE   90     // servo corredera: agarrar sopa
#define CORREDERA_SUELTA   160    // servo corredera: soltar
#define CORREDERA_CERO     0
#define TAPADERA_CERRAR    250


// ================= DIRECCIONES / REGISTROS =================
// Solo 2 esclavos: Slave1 (corredera: IR, ultrasonico, 2 servos, motor DC)
//                  Slave2 (dispensado: temp, steppers, servo360, relay)
#define SLAVE_1  0x30
#define SLAVE_2  0x31

#define REG1_SOPA        0   // IR presencia
#define REG1_DIST        1   // ultrasonico (cm)
#define REG1_SERVO_CORR  2   // servo corredera (0-180)
#define REG1_SERVO_TAPA  3   // servo tapadera  (0-180)
#define REG1_MOTOR       4   // motor DC corredera: 0=paro,1=avanza,2=retrocede

#define REG2_TEMP        0   // HW503 (A1)
#define REG2_RELAY       2
#define REG2_SERVO360    4
#define REG2_STEP_COND   5
#define REG2_STEP_CHILE  6
#define REG2_STATUS      7

// ================= ELECTRODOS / PINES =================
#define E_EMERG   (1u<<0)
#define E_RETRO   (1u<<1)
#define E_AVANZA  (1u<<2)
#define E_FASE9   (1u<<3)
#define E_FASE8   (1u<<4)
#define E_FASE6   (1u<<5)
#define E_FASE4   (1u<<6)
#define E_FASE2   (1u<<7)
#define E_AGUA25  (1u<<8)
#define E_AGUA50  (1u<<9)
#define E_AGUA75  (1u<<10)
#define E_AGUA100 (1u<<11)

#define LED_BIT       PC1   // A1  (indicador de toque)
#define BTN_START     PC2   // A2  (pulsado = LOW)

// ================= TIMER0 = millis() (no bloqueante) =================
volatile uint32_t g_ms = 0;
ISR(TIMER0_COMPA_vect){ g_ms++; }
static uint32_t millis(void){
    uint8_t s = SREG; cli(); uint32_t m = g_ms; SREG = s; return m;
}
static void timer0_init(void){
    TCCR0A = (1<<WGM01);                 // CTC
    OCR0A  = 249;                        // 16MHz/64/250 = 1000 Hz -> 1 ms
    TIMSK0 = (1<<OCIE0A);
    TCCR0B = (1<<CS01)|(1<<CS00);        // prescaler 64
}

// ================= FASES =================
enum {
    F1_ESPERA_SOPA = 1, F2_AGARRE, F3_A_DIST1, F4_DISPENSA, F5_A_DIST2,
    F6_AGUA, F7_A_DIST3, F8_SUELTA, F9_TAPA, F10_COCCION, F11_LISTO, F12_REINICIO
};

// ---- Helpers de comando ----
static void corredera(uint8_t modo){ I2C_Master_WriteReg(SLAVE_1, REG1_MOTOR, modo); }
static void servo360 (uint8_t on)  { I2C_Master_WriteReg(SLAVE_2, REG2_SERVO360, on); }
static void servo_corr(uint8_t a)  { I2C_Master_WriteReg(SLAVE_1, REG1_SERVO_CORR, a); } // Slave1 pendiente
static void servo_tapa(uint8_t a)  { I2C_Master_WriteReg(SLAVE_1, REG1_SERVO_TAPA, a); } // Slave1 pendiente

static uint8_t convertirTempC(uint8_t crudo){
    // Calibracion 2 puntos (el crudo BAJA cuando sube la temperatura). Ajustar.
    const int16_t RA=46, TA=24, RC=29, TC=45;
    int16_t t = TA + ((int16_t)(TC-TA) * ((int16_t)crudo - RA)) / (RC - RA);
    if (t<0) t=0; if (t>99) t=99;
    return (uint8_t)t;
}
static uint8_t nivel_agua(uint16_t toques){
    uint8_t a = 0;
    if (toques & E_AGUA25)  a = 25;
    if (toques & E_AGUA50)  a = 50;
    if (toques & E_AGUA75)  a = 75;
    if (toques & E_AGUA100) a = 100;
    return a;
}

// Escribe una linea completa del LCD (rellena a 16 columnas).
static void lcd_linea(uint8_t fila, const char *texto){
    LCD16x2_8bits_set_cursor(1, fila + 1);
    uint8_t i = 0;
    for (; texto[i] && i < 16; i++) LCD16x2_8bits_char(texto[i]);
    for (; i < 16; i++) LCD16x2_8bits_char(' ');
}

// ---- DIAGNOSTICO: escaneo I2C por UART (una vez al arranque) ----
// Corre ANTES de la telemetria "#..." para no confundir al ESP32.
static void writeHex2(uint8_t v){
    char buf[3];
    uint8_t hi = (v >> 4) & 0xF, lo = v & 0xF;
    buf[0] = hi < 10 ? ('0' + hi) : ('A' + hi - 10);
    buf[1] = lo < 10 ? ('0' + lo) : ('A' + lo - 10);
    buf[2] = 0;
    writeTextUART(buf);
}
static void escanear_i2c(void){
    writeTextUART("--- escaneo I2C ---\r\n");
    uint8_t encontrados = 0;
    for (uint8_t addr = 1; addr < 127; addr++){
        uint8_t ack = 0;
        if (I2C_Master_Start()){
            ack = (I2C_Master_Write((addr << 1) | 0) == 0x18);
        }
        I2C_Master_Stop();
        if (ack){
            writeTextUART("Responde: 0x"); writeHex2(addr); writeTextUART("\r\n");
            encontrados++;
        }
    }
    if (!encontrados) writeTextUART("Nada respondio.\r\n");
    writeTextUART("-------------------\r\n");
}

int main(void){
    DDRC |= (1<<LED_BIT);                       // A1 salida (LED toque)
    DDRC &= ~(1<<BTN_START);   // A2 entrada
    PORTC |= (1<<BTN_START);   // pull-up

    initUART9600();
    I2C_Master_Init(100000, 1);
    MPR121_Init();
    LCD16x2_8bits_init();
    Servo360_Init();
    timer0_init();
    sei();

    escanear_i2c();   // diagnostico unico al arranque (antes de la telemetria)

    uint8_t  fase = F12_REINICIO, fase_prev = 0;
    uint8_t  emergencia = 0, paso = 0;
    uint32_t t_ref = 0;
    uint16_t toques_prev = 0;

    lcd_linea(0, "Presione START");
    lcd_linea(1, "para iniciar");

    while (1){
        // ---------- Lecturas ----------
        uint16_t toques = MPR121_LeerTouch();
        uint16_t flanco = toques & ~toques_prev;   // electrodos recien tocados
        if (toques) PORTC |= (1<<LED_BIT); else PORTC &= ~(1<<LED_BIT);

        // ---------- Comandos por UART (desde el ESP32 / Adafruit) ----------
        uint8_t cmd_paro=0, cmd_reset=0, cmd_avanza=0, cmd_retro=0;
        uint8_t cmd_f2=0, cmd_f4=0, cmd_f6=0, cmd_f8=0, cmd_f9=0;
        int16_t c;
        while ((c = readUART()) >= 0){
            switch ((char)c){
                case 'P': cmd_paro=1;   break;   // paro emergencia
                case 'R': cmd_reset=1;  break;   // reset paro
                case 'A': cmd_avanza=1; break;   // avanzar corredera
                case 'B': cmd_retro=1;  break;   // retroceder corredera
                case '2': cmd_f2=1;     break;   // saltar a fase 2
                case '4': cmd_f4=1;     break;
                case '6': cmd_f6=1;     break;
                case '8': cmd_f8=1;     break;
                case '9': cmd_f9=1;     break;
                default: break;                  // 'S' u otros: sin accion
            }
        }

        uint8_t tempRaw = 0;
        I2C_Master_ReadReg(SLAVE_2, REG2_TEMP, &tempRaw, 1);   // HW503 (A1)
        uint8_t status2 = 0; I2C_Master_ReadReg(SLAVE_2, REG2_STATUS, &status2, 1);
        uint8_t tempC = convertirTempC(tempRaw);

        // Slave1 (pendiente): si no responde -> sopa=0, dist=255
        uint8_t sopa = 0, dist = 255, s1[2];
        if (I2C_Master_ReadReg(SLAVE_1, REG1_SOPA, s1, 2)){ sopa = s1[0]; dist = s1[1]; }

        uint8_t agua = nivel_agua(toques);
        uint8_t btnStart = !(PINC & (1<<BTN_START));   // pulsado = 1

        // ---------- Paro de emergencia (activa: IN0 o 'P';  quita: 'R') ----------
        if (!emergencia && ((toques & E_EMERG) || cmd_paro)) emergencia = 1;
        else if (emergencia && cmd_reset && !(toques & E_EMERG) && !cmd_paro) emergencia = 0;

        if (emergencia){
            corredera(0);
            servo360(0);
            I2C_Master_WriteReg(SLAVE_2, REG2_RELAY, 1);   // relay activo en paro
            lcd_linea(0, "PARO DE");
            lcd_linea(1, "EMERGENCIA");
            toques_prev = toques;
            _delay_ms(20);
            continue;
        }
        // Operacion normal: relay apagado
        I2C_Master_WriteReg(SLAVE_2, REG2_RELAY, 0);

        // ---------- Corredera manual (electrodos IN1/IN2 o UART), solo en fases inactivas ----------
        if (fase == F1_ESPERA_SOPA || fase == F12_REINICIO){
            if      ((toques & E_AVANZA) || cmd_avanza) corredera(1);
            else if ((toques & E_RETRO)  || cmd_retro)  corredera(2);
            else                                        corredera(0);
        }

        // Saltos de fase manuales (electrodos o UART/Adafruit)
        if ((flanco & E_FASE2) || cmd_f2) fase = F2_AGARRE;
        if ((flanco & E_FASE4) || cmd_f4) fase = F4_DISPENSA;
        if ((flanco & E_FASE6) || cmd_f6) fase = F6_AGUA;
        if ((flanco & E_FASE8) || cmd_f8) fase = F8_SUELTA;
        if ((flanco & E_FASE9) || cmd_f9) fase = F9_TAPA;

        // ---------- Entrada de fase (acciones que se ejecutan una sola vez) ----------
        uint8_t entrada = (fase != fase_prev);
        if (entrada){ fase_prev = fase; paso = 0; t_ref = millis(); }

        switch (fase){
        case F1_ESPERA_SOPA:
            if (entrada) corredera(0);
            if (sopa) fase = F2_AGARRE;           // Slave1 dice que hay sopa
            break;

        case F2_AGARRE:
            if (entrada) servo_corr(CORREDERA_AGARRE);   // 90 grados (Slave1)
            if (millis() - t_ref >= SETTLE_MS) fase = F3_A_DIST1;
            break;

        case F3_A_DIST1:
            if (entrada) corredera(1);            // avanzar
            if (dist <= Distancia_1){ corredera(0); fase = F4_DISPENSA; }
            break;

        case F4_DISPENSA:
            if (entrada){
                corredera(0);
                I2C_Master_WriteReg(SLAVE_2, REG2_STEP_COND, VUELTAS_CONDIMENTO);
                if (DISPENSAR_CHILE) I2C_Master_WriteReg(SLAVE_2, REG2_STEP_CHILE, VUELTAS_CHILE);
            }
            // esperar a que arranquen (>=500ms) y luego a que terminen (busy==0)
            if (millis() - t_ref >= 500 && status2 == 0) fase = F5_A_DIST2;
            break;

        case F5_A_DIST2:
            if (entrada) corredera(1);
            if (dist <= Distancia_2){ corredera(0); fase = F6_AGUA; }
            break;

        case F6_AGUA:
            if (entrada){ servo360(1); paso = 0; t_ref = millis(); }
            if (paso == 0 && millis() - t_ref >= SERVO360_MS){
                servo360(0);                      // corta la cafetera tras 2s
                paso = 1; t_ref = millis();
            }
            if (paso == 1 && millis() - t_ref >= (uint32_t)Segundos_agua * 1000UL){
                fase = F7_A_DIST3;                // espera de agua cumplida
            }
            break;

        case F7_A_DIST3:
            if (entrada) corredera(1);
            if (dist <= Distancia_3){ corredera(0); fase = F8_SUELTA; }
            break;

        case F8_SUELTA:
            if (entrada) servo_corr(CORREDERA_SUELTA);   // 160 grados
            if (millis() - t_ref >= SETTLE_MS) fase = F9_TAPA;
            break;

        case F9_TAPA:
            if (entrada){ servo_corr(CORREDERA_CERO); servo_tapa(TAPADERA_CERRAR); }
            if (millis() - t_ref >= SETTLE_MS) fase = F10_COCCION;
            break;

        case F10_COCCION:
            if (millis() - t_ref >= MIN3_MS) fase = F11_LISTO;
            break;

        case F11_LISTO:
            fase = F12_REINICIO;
            break;

        case F12_REINICIO:
        default:
            if (entrada) corredera(0);
            if (btnStart) fase = F1_ESPERA_SOPA;  // START reinicia el proceso
            break;
        }

        // ---------- LCD ----------
        if (fase == F11_LISTO || (fase == F12_REINICIO && fase_prev == F11_LISTO)){
            lcd_linea(0, "Sopa lista!");
            lcd_linea(1, "Presione START");
        } else if (fase == F12_REINICIO){
            lcd_linea(0, "Presione START");
            lcd_linea(1, "para iniciar");
        } else {
            char l0[17]; uint8_t i=0;
            const char *p = "Fase "; while(*p) l0[i++]=*p++;
            if (fase>=10){ l0[i++]='1'; l0[i++]='0'+(fase-10); } else l0[i++]='0'+fase;
            l0[i++]=' '; l0[i++]='T'; l0[i++]=':';
            if (tempC>=10) l0[i++]='0'+tempC/10;
            l0[i++]='0'+tempC%10; l0[i++]='C'; l0[i]=0;
            lcd_linea(0, l0);

            char l1[17]; i=0;
            const char *q = "Agua:"; while(*q) l1[i++]=*q++;
            if (agua>=100){ l1[i++]='1'; l1[i++]='0'; l1[i++]='0'; }
            else { if(agua>=10) l1[i++]='0'+agua/10; l1[i++]='0'+agua%10; }
            l1[i++]='%'; l1[i]=0;
            lcd_linea(1, l1);
        }

        // ---------- Telemetria al ESP32 (linea compacta, la parsea el ESP32) ----------
        //   #T:<temp>,A:<agua>,D:<dist>,F:<fase>,E:<emerg>,S:<sopa>
        writeTextUART("#T:"); writeNumberUART(tempC);
        writeTextUART(",A:"); writeNumberUART(agua);
        writeTextUART(",D:"); writeNumberUART(dist);
        writeTextUART(",F:"); writeNumberUART(fase);
        writeTextUART(",E:"); writeNumberUART(emergencia);
        writeTextUART(",S:"); writeNumberUART(sopa);
        writeTextUART("\r\n");

        toques_prev = toques;
        _delay_ms(20);
    }
}
