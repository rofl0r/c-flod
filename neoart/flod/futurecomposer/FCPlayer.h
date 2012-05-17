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

#define FCPLAYER_SEQS_BUFFERSIZE 64
#define FCPLAYER_PATS_BUFFERSIZE 64
#define FCPLAYER_VOLS_BUFFERSIZE 64
#define FCPLAYER_FRQS_BUFFERSIZE 64

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
	struct ByteArray seqs_struct;
	struct ByteArray pats_struct;
	struct ByteArray vols_struct;
	struct ByteArray frqs_struct;
	unsigned char seqs_buffer[FCPLAYER_SEQS_BUFFERSIZE];
	unsigned char pats_buffer[FCPLAYER_PATS_BUFFERSIZE];
	unsigned char vols_buffer[FCPLAYER_VOLS_BUFFERSIZE];
	unsigned char frqs_buffer[FCPLAYER_FRQS_BUFFERSIZE];
	off_t seqs_used;
	off_t pats_used;
	off_t vols_used;
	off_t frqs_used;
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
