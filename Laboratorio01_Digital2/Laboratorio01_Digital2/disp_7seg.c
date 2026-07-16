/*
 * disp_7seg.c
 *
 * Implementacion de la libreria de display de 7 segmentos con pines
 * configurables individualmente. Ver disp_7seg.h para el uso.
 */

#include "disp_7seg.h"

// Guarda, para UN segmento, en que puerto y pin esta conectado.
// ddr: registro de direccion (DDRx) de ese puerto, para configurarlo como salida
// port: registro de salida (PORTx) de ese puerto, para poner el pin en alto/bajo
// pin: numero de bit dentro de ese puerto (0-7)

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    uint8_t pin;
} seg_pin_t;

// Nombramos numeros para segmentos, asi se facilita la lectura del codigo.
#define SEG_A  0
#define SEG_B  1
#define SEG_C  2
#define SEG_D  3
#define SEG_E  4
#define SEG_F  5
#define SEG_G  6
#define SEG_DP 7
#define NUM_SEGMENTOS 8

// Arreglo con la info de los 8 segmentos. Se llena una sola vez en
// disp7seg_init() y despues las demas funciones lo usan para saber
// que pin mover.
static seg_pin_t segmentos[NUM_SEGMENTOS];

// Tabla de bits para encender cada numero en un display de 7 segmentos anodo comun.
static const uint8_t tabla_digitos[16] = {
    0b11111100, // 0
    0b01100000, // 1
    0b11011010, // 2
    0b11110010, // 3
    0b01100110, // 4
    0b10110110, // 5
    0b10111110, // 6
    0b11100000, // 7
    0b11111110, // 8
    0b11110110, // 9
    0b11101110, // A
    0b00111110, // b
    0b10011100, // C
    0b01111010, // d
    0b10011110, // E
    0b10001110  // F
};

// Guarda el puerto y pin de UN segmento dentro del arreglo "segmentos".
// indice: cual segmento es

static void configurar_pin(uint8_t indice, volatile uint8_t *port, uint8_t pin)
{
    segmentos[indice].port = port;
    segmentos[indice].pin  = pin;

    // Truco para no pedir tambien el registro DDR como parametro:
    // en el ATmega328P los registros de cada puerto estan en este orden
    segmentos[indice].ddr  = port - 1;
}

// Enciende o apaga UN segmento en su pin correspondiente.

static void escribir_segmento(uint8_t indice, uint8_t encendido)
{
    if (encendido) {
        // Poner el bit del pin en 0 sin tocar los demas bits del puerto
        *(segmentos[indice].port) &= (uint8_t)~(1 << segmentos[indice].pin);
    } else {
        // Poner el bit del pin en 1 sin tocar los demas bits del puerto
        *(segmentos[indice].port) |= (uint8_t)(1 << segmentos[indice].pin);
    }
}

// Funcion que se llama una sola vez, al inicio del programa, para decirle
// a la libreria en que puerto/pin quedo conectado cada segmento.
void disp7seg_init(volatile uint8_t *portA, uint8_t pinA,
                    volatile uint8_t *portB, uint8_t pinB,
                    volatile uint8_t *portC, uint8_t pinC,
                    volatile uint8_t *portD, uint8_t pinD,
                    volatile uint8_t *portE, uint8_t pinE,
                    volatile uint8_t *portF, uint8_t pinF,
                    volatile uint8_t *portG, uint8_t pinG,
                    volatile uint8_t *portDP, uint8_t pinDP)
{
    // Guardar en el arreglo "segmentos" donde quedo cada uno
    configurar_pin(SEG_A,  portA,  pinA);
    configurar_pin(SEG_B,  portB,  pinB);
    configurar_pin(SEG_C,  portC,  pinC);
    configurar_pin(SEG_D,  portD,  pinD);
    configurar_pin(SEG_E,  portE,  pinE);
    configurar_pin(SEG_F,  portF,  pinF);
    configurar_pin(SEG_G,  portG,  pinG);
    configurar_pin(SEG_DP, portDP, pinDP);

    // Configurar cada pin usado como salida (poner su bit en 1 dentro de DDRx)
    for (uint8_t i = 0; i < NUM_SEGMENTOS; i++) {
        *(segmentos[i].ddr) |= (uint8_t)(1 << segmentos[i].pin);
    }

    // Dejar el display apagado al arrancar
    disp7seg_apagar();
}

// Muestra un digito (0-9, o 10-15 para A-F) en el display.
// Recorre los 7 segmentos (A-G, sin contar el DP) y prende/apaga cada
// uno segun el bit correspondiente en la tabla de digitos.
void displ_7seg(uint8_t num)
{
    if (num > 15) {
        num = 15; // por seguridad, si mandan un numero fuera de rango
    }

    uint8_t patron = tabla_digitos[num];

    // i = 0 -> segmento A (bit 7 del patron), i = 6 -> segmento G (bit 1)
    for (uint8_t i = 0; i < NUM_SEGMENTOS - 1; i++) {
        uint8_t encendido = (patron >> (7 - i)) & 0x01;
        escribir_segmento(i, encendido);
    }
}

// Prende o apaga el punto decimal, aparte del digito principal.
void disp7seg_dp(uint8_t encendido)
{
    escribir_segmento(SEG_DP, encendido ? 1 : 0);
}

// Apaga los 8 segmentos (util al iniciar o entre un digito y otro).
void disp7seg_apagar(void)
{
    for (uint8_t i = 0; i < NUM_SEGMENTOS; i++) {
        escribir_segmento(i, 0);
    }
}
