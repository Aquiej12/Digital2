/*
 * songs.c
 * Aquí (y solo aquí) se incluyen los .h de las canciones: así cada canción existe una sola vez en flash.
 */
#include "songs.h"
#include "synth.h"
#include "audio.h"
#include "main.h"

#include "song_demo.h"
#include "song_seven_nation_army.h"
#include "song_b_e_r___the_night_begins_to_shine__wip.h"
#include "song_mighty_morphin_power_rangers.h"
#include "song_michael_jackson___beat_it.h"
#include "song_pokemon_black_and_white_low_hp_midi_version.h"

/* Fila de una canción exportada CON canales (el conversor genera prefijo_canal1 y su _N) */
#define CON_JUGADOR1(song, canal1, n, master)  { &(song), (canal1), (n), (master) }
/* Fila de una canción SIN canales (no se mutea nada al soltar el botón) */
#define SIN_JUGADOR1(song, master)             { &(song), 0, 0, (master) }

/* Cada fila va en el lugar de su nombre (no importa el orden en que las escribas) */
const SongEntry song_table[CANCION_TOTAL] = {
	[CANCION_DEMO]                  = SIN_JUGADOR1(song_demo, 0),
	[CANCION_SEVEN_NATION_ARMY]     = CON_JUGADOR1(song_seven_nation_army,
	                                      seven_nation_army_canal1, SONG_SEVEN_NATION_ARMY_CANAL1_N, 0),
	[CANCION_NIGHT_BEGINS_TO_SHINE] = CON_JUGADOR1(song_b_e_r___the_night_begins_to_shine__wip,
	                                      b_e_r___the_night_begins_to_shine__wip_canal1,
	                                      SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_CANAL1_N, 0),
	[CANCION_POWER_RANGERS]         = SIN_JUGADOR1(song_powerrangers, 0),
	[CANCION_BEAT_IT]               = CON_JUGADOR1(song_michael_jackson___beat_it,
	                                      michael_jackson___beat_it_canal1,
	                                      SONG_MICHAEL_JACKSON___BEAT_IT_CANAL1_N,
	                                      160),   /* 16 pistas: con 224 recorta */
	[CANCION_POKEMON]               = CON_JUGADOR1(song_pokemon_black_and_white_low_hp_midi_version,
	                                      pokemon_black_and_white_low_hp_midi_version_canal1,
	                                      SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_CANAL1_N,
	                                      170),   /* con 224 recorta un poco */
};

volatile uint8_t cancion_seleccionada = CANCION_NINGUNA;

static uint8_t cancion_sonando = CANCION_NINGUNA;   /* interno: lo que de verdad está sonando */

uint8_t Canciones_Actualizar(void) {
	uint8_t pedida = cancion_seleccionada;          /* una sola lectura (puede cambiarla una IRQ) */
	if (pedida == cancion_sonando) return 0;

	if (pedida >= CANCION_TOTAL) {                  /* CANCION_NINGUNA o número inválido: silencio */
		HAL_NVIC_DisableIRQ(DMA1_Stream5_IRQn);
		Music_Stop();
		HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
		cancion_sonando = CANCION_NINGUNA;
		return 1;
	}

	const SongEntry *e = &song_table[pedida];
	Audio_SetMasterVolume(e->master ? e->master : AUDIO_MASTER_VOLUME);
	Audio_PlaySong(e->song, 1);                     /* 1 = repetir */
	cancion_sonando = pedida;
	return 1;
}

const SongEntry *Canciones_Actual(void) {
	return (cancion_sonando < CANCION_TOTAL) ? &song_table[cancion_sonando] : 0;
}
