#ifndef FXPLAYER_H
#define FXPLAYER_H

#include "../core/Amiga.h"
#include "../core/AmigaRow.h"
#include "../core/AmigaSample.h"
#include "../core/AmigaPlayer.h"

#include "FXVoice.h"

enum FXPlayerVersion {
	SOUNDFX_10 = 1,
	SOUNDFX_18 = 2,
	SOUNDFX_19 = 3,
	SOUNDFX_20 = 4,
};

#define FXPLAYER_MAX_ROWS 4
#define FXPLAYER_MAX_SAMPLES 4

//fixed
#define FXPLAYER_MAX_VOICES 4
#define FXPLAYER_MAX_TRACKS 128

struct FXPlayer {
	struct AmigaPlayer super;
	//track      : Vector.<int>,
	int track[FXPLAYER_MAX_TRACKS];
	//patterns   : Vector.<AmigaRow>,
	struct AmigaRow patterns[FXPLAYER_MAX_ROWS];
	//samples    : Vector.<AmigaSample>,
	struct AmigaSample samples[FXPLAYER_MAX_SAMPLES];
	int length;
	//voices     : Vector.<FXVoice>,
	struct FXVoice voices[FXPLAYER_MAX_VOICES];
	int trackPos;
	int patternPos;
	int jumpFlag;
	int delphine;
};

void FXPlayer_defaults(struct FXPlayer* self);
void FXPlayer_ctor(struct FXPlayer* self, struct Amiga *amiga);
struct FXPlayer* FXPlayer_new(struct Amiga *amiga);

#endif
