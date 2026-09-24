/*
 * seven_nation_army.h
 * Seven Nation Army — The White Stripes (riff principal)
 *
 * Riff: E  E  G  E  D  C  B   (× 3)
 *
 * Basado en las duraciones del Beep() de Windows que suenan correctas.
 *
 * Para Lab 7 Post Lab - IE3054 Electrónica Digital 2, UVG
 */

#ifndef SEVEN_NATION_ARMY_H
#define SEVEN_NATION_ARMY_H

#include "notas.h"

/* ---- Cantidad de notas ---- */
#define SNA_LEN  24

/* ---- Frecuencias de cada nota ---- */
const uint16_t sna_notas[SNA_LEN] = {

    /* Riff 1 */
    NOTE_E4,  NOTE_E4,  NOTE_G4,  NOTE_E4,
    NOTE_D4,  NOTE_C4,  NOTE_B3,  REST,

    /* Riff 2 */
    NOTE_E4,  NOTE_E4,  NOTE_G4,  NOTE_E4,
    NOTE_D4,  NOTE_C4,  NOTE_B3,  REST,

    /* Riff 3 */
    NOTE_E4,  NOTE_E4,  NOTE_G4,  NOTE_E4,
    NOTE_D4,  NOTE_C4,  NOTE_B3,  REST,
};

/* ---- Duración de cada nota (en ms) ---- */
const uint16_t sna_duraciones[SNA_LEN] = {

    /* Riff 1 */
    500,  200,  350,  350,
    350,  550,  600,  400,

    /* Riff 2 */
    500,  200,  350,  350,
    350,  550,  600,  400,

    /* Riff 3 */
    500,  200,  350,  350,
    350,  550,  600,  400,
};

#endif /* SEVEN_NATION_ARMY_H */
