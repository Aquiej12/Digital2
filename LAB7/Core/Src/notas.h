/*
 * notas.h
 * Frecuencias de todas las notas musicales (temperamento igual, A4 = 440 Hz)
 * Cubre desde C0 hasta B8 — más que un piano completo de 88 teclas.
 *
 * Convención de nombres:
 *   NOTE_C4  = Do central (261 Hz)
 *   NOTE_Cs4 = Do sostenido 4 / Re bemol 4
 *   NOTE_Db4 = alias de NOTE_Cs4
 *
 * REST = 0  → silencio (el reproductor debe apagar el PWM)
 *
 * Para Lab 7 - IE3054 Electrónica Digital 2, UVG
 */

#ifndef NOTAS_H
#define NOTAS_H

#define REST    0

/* ============ Octava 0 ============ */
#define NOTE_C0     16
#define NOTE_Cs0    17
#define NOTE_Db0    17
#define NOTE_D0     18
#define NOTE_Ds0    19
#define NOTE_Eb0    19
#define NOTE_E0     21
#define NOTE_F0     22
#define NOTE_Fs0    23
#define NOTE_Gb0    23
#define NOTE_G0     25
#define NOTE_Gs0    26
#define NOTE_Ab0    26
#define NOTE_A0     28
#define NOTE_As0    29
#define NOTE_Bb0    29
#define NOTE_B0     31

/* ============ Octava 1 ============ */
#define NOTE_C1     33
#define NOTE_Cs1    35
#define NOTE_Db1    35
#define NOTE_D1     37
#define NOTE_Ds1    39
#define NOTE_Eb1    39
#define NOTE_E1     41
#define NOTE_F1     44
#define NOTE_Fs1    46
#define NOTE_Gb1    46
#define NOTE_G1     49
#define NOTE_Gs1    52
#define NOTE_Ab1    52
#define NOTE_A1     55
#define NOTE_As1    58
#define NOTE_Bb1    58
#define NOTE_B1     62

/* ============ Octava 2 ============ */
#define NOTE_C2     65
#define NOTE_Cs2    69
#define NOTE_Db2    69
#define NOTE_D2     73
#define NOTE_Ds2    78
#define NOTE_Eb2    78
#define NOTE_E2     82
#define NOTE_F2     87
#define NOTE_Fs2    93
#define NOTE_Gb2    93
#define NOTE_G2     98
#define NOTE_Gs2    104
#define NOTE_Ab2    104
#define NOTE_A2     110
#define NOTE_As2    117
#define NOTE_Bb2    117
#define NOTE_B2     123

/* ============ Octava 3 ============ */
#define NOTE_C3     131
#define NOTE_Cs3    139
#define NOTE_Db3    139
#define NOTE_D3     147
#define NOTE_Ds3    156
#define NOTE_Eb3    156
#define NOTE_E3     165
#define NOTE_F3     175
#define NOTE_Fs3    185
#define NOTE_Gb3    185
#define NOTE_G3     196
#define NOTE_Gs3    208
#define NOTE_Ab3    208
#define NOTE_A3     220
#define NOTE_As3    233
#define NOTE_Bb3    233
#define NOTE_B3     247

/* ============ Octava 4 (Do central = C4) ============ */
#define NOTE_C4     262
#define NOTE_Cs4    277
#define NOTE_Db4    277
#define NOTE_D4     294
#define NOTE_Ds4    311
#define NOTE_Eb4    311
#define NOTE_E4     330
#define NOTE_F4     349
#define NOTE_Fs4    370
#define NOTE_Gb4    370
#define NOTE_G4     392
#define NOTE_Gs4    415
#define NOTE_Ab4    415
#define NOTE_A4     440
#define NOTE_As4    466
#define NOTE_Bb4    466
#define NOTE_B4     494

/* ============ Octava 5 ============ */
#define NOTE_C5     523
#define NOTE_Cs5    554
#define NOTE_Db5    554
#define NOTE_D5     587
#define NOTE_Ds5    622
#define NOTE_Eb5    622
#define NOTE_E5     659
#define NOTE_F5     698
#define NOTE_Fs5    740
#define NOTE_Gb5    740
#define NOTE_G5     784
#define NOTE_Gs5    831
#define NOTE_Ab5    831
#define NOTE_A5     880
#define NOTE_As5    932
#define NOTE_Bb5    932
#define NOTE_B5     988

/* ============ Octava 6 ============ */
#define NOTE_C6     1047
#define NOTE_Cs6    1109
#define NOTE_Db6    1109
#define NOTE_D6     1175
#define NOTE_Ds6    1245
#define NOTE_Eb6    1245
#define NOTE_E6     1319
#define NOTE_F6     1397
#define NOTE_Fs6    1480
#define NOTE_Gb6    1480
#define NOTE_G6     1568
#define NOTE_Gs6    1661
#define NOTE_Ab6    1661
#define NOTE_A6     1760
#define NOTE_As6    1865
#define NOTE_Bb6    1865
#define NOTE_B6     1976

/* ============ Octava 7 ============ */
#define NOTE_C7     2093
#define NOTE_Cs7    2217
#define NOTE_Db7    2217
#define NOTE_D7     2349
#define NOTE_Ds7    2489
#define NOTE_Eb7    2489
#define NOTE_E7     2637
#define NOTE_F7     2794
#define NOTE_Fs7    2960
#define NOTE_Gb7    2960
#define NOTE_G7     3136
#define NOTE_Gs7    3322
#define NOTE_Ab7    3322
#define NOTE_A7     3520
#define NOTE_As7    3729
#define NOTE_Bb7    3729
#define NOTE_B7     3951

/* ============ Octava 8 ============ */
#define NOTE_C8     4186
#define NOTE_Cs8    4435
#define NOTE_Db8    4435
#define NOTE_D8     4699
#define NOTE_Ds8    4978
#define NOTE_Eb8    4978
#define NOTE_E8     5274
#define NOTE_F8     5588
#define NOTE_Fs8    5920
#define NOTE_Gb8    5920
#define NOTE_G8     6272
#define NOTE_Gs8    6645
#define NOTE_Ab8    6645
#define NOTE_A8     7040
#define NOTE_As8    7459
#define NOTE_Bb8    7459
#define NOTE_B8     7902

#endif /* NOTAS_H */
