/*
 * song_b_e_r___the_night_begins_to_shine__wip.h
 * Generado por midi_to_c.py desde B.E.R - The Night Begins to Shine (WIP).mid
 * Incluir en UN solo .c. Los arreglos son static const (flash).
 */
#ifndef SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_H_
#define SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_H_

#include "music.h"
#include "synth.h"

#define SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_BPM 130

#include "song_b_e_r___the_night_begins_to_shine__wip_bass_guitar.h"
#include "song_b_e_r___the_night_begins_to_shine__wip_distortion_guitar.h"
#include "song_b_e_r___the_night_begins_to_shine__wip_electric_guitar.h"
#include "song_b_e_r___the_night_begins_to_shine__wip_synth_pluck.h"
#include "song_b_e_r___the_night_begins_to_shine__wip_electric_drum_kit.h"

static const TrackDef b_e_r___the_night_begins_to_shine__wip_tracks[] = {
	TRACK(b_e_r___the_night_begins_to_shine__wip_bass_guitar, INST_BASS, VOL(0.30)),
	TRACK(b_e_r___the_night_begins_to_shine__wip_distortion_guitar, INST_GUITAR, VOL(0.30)),
	TRACK(b_e_r___the_night_begins_to_shine__wip_electric_guitar, INST_GUITAR, VOL(0.30)),
	TRACK(b_e_r___the_night_begins_to_shine__wip_synth_pluck, INST_SYNTH, VOL(0.30)),
	TRACK(b_e_r___the_night_begins_to_shine__wip_electric_drum_kit, INST_DRUMS, VOL(0.30)),
};

static const Song song_b_e_r___the_night_begins_to_shine__wip = { "b_e_r___the_night_begins_to_shine__wip", SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_BPM, 5, b_e_r___the_night_begins_to_shine__wip_tracks };

#endif /* SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_H_ */
