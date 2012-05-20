#ifndef PTPLAYER_H
#define PTPLAYER_H

#include "PTRow.h"
#include "PTSample.h"
#include "PTVoice.h"

#include "../core/Amiga.h"

#define PTPLAYER_MAX_TRACKS 128
#define PTPLAYER_MAX_SAMPLES 32
#define PTPLAYER_MAX_VOICES 4

#define PTPLAYER_MAX_PATTERNS 16


struct PTPlayer {
	int track[PTPLAYER_MAX_TRACKS];//Vector.<int>,
	struct PTRow patterns[PTPLAYER_MAX_PATTERNS]; //Vector.<PTRow>,
	struct PTSample samples[PTPLAYER_MAX_SAMPLES];//Vector.<PTSample>,
	int length;
	struct PTVoice voices[PTPLAYER_MAX_VOICES];//Vector.<PTVoice>,
	int trackPos;
	int patternPos;
	int patternBreak;
	int patternDelay;
	int breakPos;
	int jumpFlag;
	int vibratoDepth;
};

void PTPlayer_defaults(struct PTPlayer* self);
void PTPlayer_ctor(struct PTPlayer* self, struct Amiga *amiga);
struct PTPlayer* PTPlayer_new(struct Amiga *amiga);

#endif
