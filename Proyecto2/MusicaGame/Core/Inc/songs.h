/*
 * songs.h
 * Lista de canciones y selección de la que suena.
 *
 * ELEGIR CANCIÓN:   cancion_seleccionada = CANCION_BEAT_IT;      (cualquier momento, desde cualquier parte)
 * DETENER:          cancion_seleccionada = CANCION_NINGUNA;
 * En el while(1) va Canciones_Actualizar(): ve el cambio y arranca / detiene la música.
 * Después, la comunicación (UART/SPI/I2C) solo tiene que escribir el número en cancion_seleccionada.
 *
 * AGREGAR UNA CANCIÓN:
 *   1. Copia sus .h (song_x.h y song_x_canalN.h) a Core/Inc.
 *   2. Aquí: agrega su nombre a la lista, ANTES de CANCION_TOTAL.
 *   3. En songs.c: #include "song_x.h" y su fila en la tabla.
 */
#ifndef INC_SONGS_H_
#define INC_SONGS_H_

#include <stdint.h>
#include "music.h"

/* El número de cada canción es su posición en esta lista (0, 1, 2...) */
enum {
	CANCION_DEMO = 0,
	CANCION_SEVEN_NATION_ARMY,         /* 1 */
	CANCION_NIGHT_BEGINS_TO_SHINE,     /* 2 */
	CANCION_POWER_RANGERS,             /* 3 */
	CANCION_BEAT_IT,                   /* 4 */
	CANCION_POKEMON,                   /* 5 */
	/* ...agrega aquí las nuevas... */
	CANCION_TOTAL,                     /* cuántas hay (no es una canción) */
	CANCION_NINGUNA = 0xFF             /* silencio */
};

typedef struct {
	const Song    *song;
	const uint8_t *p1_tracks;   /* pistas del jugador 1 (canal 1); 0 si la canción no tiene canales */
	uint8_t        p1_n;        /* cuántas pistas tiene el jugador 1 */
	uint16_t       master;      /* volumen maestro para esta canción (0 = el normal, 224) */
} SongEntry;

extern const SongEntry song_table[CANCION_TOTAL];

/* La canción que se quiere oír. Se puede cambiar desde el código, el depurador o una interrupción. */
extern volatile uint8_t cancion_seleccionada;

/* Llamar en el while(1). Devuelve 1 cuando acaba de cambiar de canción (o de detenerse). */
uint8_t Canciones_Actualizar(void);

/* Fila de la canción que está sonando, o 0 si no suena ninguna */
const SongEntry *Canciones_Actual(void);

#endif /* INC_SONGS_H_ */
