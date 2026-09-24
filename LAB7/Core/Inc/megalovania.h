/*
 * megalovania.h
 * Megalovania — Undertale (Toby Fox)
 *
 * Estructura modular:
 *   megalovania_notas[]      → frecuencias (usa defines de notas.h)
 *   megalovania_duraciones[]  → duración de cada nota en ms
 *   MEGALOVANIA_LEN           → cantidad de notas
 *
 * BPM = 120
 *
 * Para Lab 7 - IE3054 Electrónica Digital 2, UVG
 */

#ifndef MEGALOVANIA_H
#define MEGALOVANIA_H

#include "notas.h"

/* ---- Duraciones en ms (BPM = 120) ---- */
#define MEGA_BPM    120
#define MEGA_W      (4 * 60000 / MEGA_BPM)      /* Redonda       = 2000 ms */
#define MEGA_H      (2 * 60000 / MEGA_BPM)      /* Blanca        = 1000 ms */
#define MEGA_Q      (60000 / MEGA_BPM)           /* Negra         =  500 ms */
#define MEGA_E      (60000 / MEGA_BPM / 2)       /* Corchea       =  250 ms */
#define MEGA_S      (60000 / MEGA_BPM / 4)       /* Semicorchea   =  125 ms */
#define MEGA_QD     (MEGA_Q + MEGA_E)            /* Negra con punto = 750 ms */
#define MEGA_HD     (MEGA_H + MEGA_Q)            /* Blanca con punto = 1500 ms */
#define MEGA_ED     (MEGA_E + MEGA_S)            /* Corchea con punto = 375 ms */

/* ---- Cantidad de notas ---- */
#define MEGALOVANIA_LEN  68

/* ---- Frecuencias de cada nota ---- */
const uint16_t megalovania_notas[MEGALOVANIA_LEN] = {
    /* Frase 1: D D D(oct) A */
    NOTE_D4,  NOTE_D4,  NOTE_D5,  NOTE_A4,
    /* Gs Ab G F */
    REST,     NOTE_Gs4, NOTE_G4,  NOTE_F4,
    /* D F G */
    NOTE_D4,  NOTE_F4,  NOTE_G4,
    /* Frase 2: C C D(oct) A */
    NOTE_C4,  NOTE_C4,  NOTE_D5,  NOTE_A4,
    /* Gs Ab G F */
    REST,     NOTE_Gs4, NOTE_G4,  NOTE_F4,
    /* D F G */
    NOTE_D4,  NOTE_F4,  NOTE_G4,
    /* Frase 3: B3 B3 D(oct) A */
    NOTE_B3,  NOTE_B3,  NOTE_D5,  NOTE_A4,
    /* Gs Ab G F */
    REST,     NOTE_Gs4, NOTE_G4,  NOTE_F4,
    /* D F G */
    NOTE_D4,  NOTE_F4,  NOTE_G4,
    /* Frase 4: Bb3 Bb3 D(oct) A */
    NOTE_Bb3, NOTE_Bb3, NOTE_D5,  NOTE_A4,
    /* Gs Ab G F */
    REST,     NOTE_Gs4, NOTE_G4,  NOTE_F4,
    /* D F G */
    NOTE_D4,  NOTE_F4,  NOTE_G4,

    /* === Sección rítmica (notas rápidas) === */
    /* F F F F F D D D */
    NOTE_F4,  NOTE_F4,  NOTE_F4,  NOTE_F4,  NOTE_F4,  NOTE_D4,  NOTE_D4,  NOTE_D4,
    /* D D D D F A A */
    NOTE_D4,  NOTE_D4,  NOTE_D4,  NOTE_D4,
    NOTE_F4,
    NOTE_A4,  NOTE_A4,
    /* D5 D5 D5 A G F E F */
    NOTE_D5,  NOTE_D5,  NOTE_D5,
    NOTE_A4,  NOTE_G4,  NOTE_F4,
    NOTE_E4,  NOTE_F4,
};

/* ---- Duración de cada nota (en ms) ---- */
const uint16_t megalovania_duraciones[MEGALOVANIA_LEN] = {
    /* Frase 1 */
    MEGA_S,   MEGA_S,   MEGA_E,   MEGA_ED,
    MEGA_S,   MEGA_E,   MEGA_E,   MEGA_E,
    MEGA_E,   MEGA_E,   MEGA_E,
    /* Frase 2 */
    MEGA_S,   MEGA_S,   MEGA_E,   MEGA_ED,
    MEGA_S,   MEGA_E,   MEGA_E,   MEGA_E,
    MEGA_E,   MEGA_E,   MEGA_E,
    /* Frase 3 */
    MEGA_S,   MEGA_S,   MEGA_E,   MEGA_ED,
    MEGA_S,   MEGA_E,   MEGA_E,   MEGA_E,
    MEGA_E,   MEGA_E,   MEGA_E,
    /* Frase 4 */
    MEGA_S,   MEGA_S,   MEGA_E,   MEGA_ED,
    MEGA_S,   MEGA_E,   MEGA_E,   MEGA_E,
    MEGA_E,   MEGA_E,   MEGA_E,

    /* Sección rítmica */
    MEGA_S,   MEGA_S,   MEGA_S,   MEGA_S,   MEGA_S,   MEGA_S,   MEGA_S,   MEGA_S,
    MEGA_S,   MEGA_S,   MEGA_S,   MEGA_S,
    MEGA_E,
    MEGA_E,   MEGA_E,
    MEGA_E,   MEGA_E,   MEGA_E,
    MEGA_E,   MEGA_E,   MEGA_E,
    MEGA_E,   MEGA_E,
};

#endif /* MEGALOVANIA_H */
