#ifndef STPLAYER_H
#define STPLAYER_H

#include "../core/AmigaPlayer.h"
#include "../core/AmigaSample.h"
#include "../core/AmigaRow.h"
#include "../core/Amiga.h"
#include "STVoice.h"

enum Soundtracker_Format {
	ULTIMATE_SOUNDTRACKER = 1,
	DOC_SOUNDTRACKER_9 = 2,
	MASTER_SOUNDTRACKER = 3,
	DOC_SOUNDTRACKER_20 = 4,
};

#define STPLAYER_MAX_TRACKS 16
#define STPLAYER_MAX_SAMPLES 64
#define STPLAYER_MAX_PATTERNS 32

// fixed
#define STPLAYER_MAX_VOICES 4

/*
inheritance
??
   -> EventDispatcher
                     ->CorePlayer
                                 ->AmigaPlayer
                                              ->STPlayer
*/
struct STPlayer {
	struct AmigaPlayer super;
	//track      : Vector.<int>,
	int track[STPLAYER_MAX_TRACKS];
	//patterns   : Vector.<AmigaRow>,
	struct AmigaRow patterns[STPLAYER_MAX_PATTERNS];
	//samples    : Vector.<AmigaSample>,
	struct AmigaSample samples[STPLAYER_MAX_SAMPLES];
	int length;
	//voices     : Vector.<STVoice>,
	struct STVoice voices[STPLAYER_MAX_VOICES];
	int trackPos;
	int patternPos;
	int jumpFlag;
};


void STPlayer_defaults(struct STPlayer* self);
void STPlayer_ctor(struct STPlayer* self, struct Amiga *amiga);
struct STPlayer* STPlayer_new(struct Amiga *amiga);


#endif
