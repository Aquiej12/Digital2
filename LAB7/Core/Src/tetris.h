/*
 * tetris.h
 * Tetris Theme A (Korobeiniki) — datos de la melodía
 *
 * Estructura modular:
 *   tetris_notas[]      → frecuencias (usa defines de notas.h)
 *   tetris_duraciones[]  → duración de cada nota en ms
 *   TETRIS_LEN           → cantidad de notas (para el reproductor)
 *
 * BPM = 144
 *
 * Para Lab 7 - IE3054 Electrónica Digital 2, UVG
 */

#ifndef TETRIS_H
#define TETRIS_H

#include "notas.h"

/* ---- Duraciones en ms (BPM = 144) ---- */
#define BPM     144
#define W       (4 * 60000 / BPM)      /* Redonda       = 1667 ms */
#define H       (2 * 60000 / BPM)      /* Blanca        =  833 ms */
#define Q       (60000 / BPM)          /* Negra         =  417 ms */
#define E_D     (60000 / BPM / 2)      /* Corchea       =  208 ms */
#define S_D     (60000 / BPM / 4)      /* Semicorchea   =  104 ms */
#define QD      (Q + E_D)              /* Negra con punto = 625 ms */
#define HD      (H + Q)                /* Blanca con punto = 1250 ms */

/* ---- Cantidad de notas ---- */
#define TETRIS_LEN  38

/* ---- Frecuencias de cada nota ---- */
const uint16_t tetris_notas[TETRIS_LEN] = {
    /* Línea 1 */
    NOTE_E5,  NOTE_B4,  NOTE_C5,  NOTE_D5,  NOTE_C5,  NOTE_B4,
    /* Línea 2 */
    NOTE_A4,  NOTE_A4,  NOTE_C5,  NOTE_E5,  NOTE_D5,  NOTE_C5,
    /* Línea 3 */
    NOTE_B4,  NOTE_C5,  NOTE_D5,  NOTE_E5,
    /* Línea 4 */
    NOTE_C5,  NOTE_A4,  NOTE_A4,
    /* Línea 5 */
    REST,     NOTE_D5,  NOTE_F5,  NOTE_A5,  NOTE_G5,  NOTE_F5,
    /* Línea 6 */
    NOTE_E5,  NOTE_C5,  NOTE_E5,  NOTE_D5,  NOTE_C5,
    /* Línea 7 */
    NOTE_B4,  NOTE_C5,  NOTE_D5,  NOTE_E5,
    /* Línea 8 */
    NOTE_C5,  NOTE_A4,  NOTE_A4,
};

/* ---- Duración de cada nota (en ms) ---- */
const uint16_t tetris_duraciones[TETRIS_LEN] = {
    /* Línea 1 */
    Q,    E_D,  E_D,  Q,    E_D,  E_D,
    /* Línea 2 */
    Q,    E_D,  E_D,  Q,    E_D,  E_D,
    /* Línea 3 */
    QD,   E_D,  Q,    Q,
    /* Línea 4 */
    Q,    Q,    H,
    /* Línea 5 */
    E_D,  Q,    E_D,  Q,    E_D,  E_D,
    /* Línea 6 */
    QD,   E_D,  Q,    E_D,  E_D,
    /* Línea 7 */
    QD,   E_D,  Q,    Q,
    /* Línea 8 */
    Q,    Q,    Q,
};

#endif /* TETRIS_H */
