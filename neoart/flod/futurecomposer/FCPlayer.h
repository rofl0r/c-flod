#ifndef FCPLAYER_H
#define FCPLAYER_H
#include "../core/AmigaPlayer.h"
#include "../core/AmigaSample.h"
#include "../core/Amiga.h"
#include "../../../flashlib/ByteArray.h"
#include "FCVoice.h"

#define FCPLAYER_SAMPLES_MAX 16
#define FCPLAYER_VOICES_MAX 4

enum Futurecomp_Version {
      FUTURECOMP_10 = 1,
      FUTURECOMP_14 = 2,
};

/*
inheritance
??
   -> EventDispatcher
                     ->CorePlayer
                                 ->AmigaPlayer
                                              ->FCPlayer
*/
struct FCPlayer {
	struct AmigaPlayer super;
	struct ByteArray *seqs;
	struct ByteArray *pats;
	struct ByteArray *vols;
	struct ByteArray *frqs;
	int length;
	// samples : Vector.<AmigaSample>,
	struct AmigaSample samples[FCPLAYER_SAMPLES_MAX];
	// voices  : Vector.<FCVoice>;
	struct FCVoice voices[FCPLAYER_VOICES_MAX];
};

extern const signed char WAVES[];
extern const unsigned short PERIODS[];

void FCPlayer_defaults(struct FCPlayer* self);
void FCPlayer_ctor(struct FCPlayer* self, struct Amiga *amiga);
struct FCPlayer* FCPlayer_new(struct Amiga *amiga);

#endif
