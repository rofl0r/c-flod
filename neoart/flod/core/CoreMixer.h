#ifndef COREMIXER_H
#define COREMIXER_H

#include "CorePlayer.h"
#include "Sample.h"
#include "../flod.h"

enum CoreMixer_type {
	CM_AMIGA,
	CM_SOUNDBLASTER,
};

/*
inheritance
object
   -> CoreMixer
*/
struct CoreMixer {
	struct CorePlayer* player;
	int samplesTick;
	// buffer      : Vector.<Sample>,
	struct Sample* buffer; //Vector
	int samplesLeft;
	int remains;
	int completed;
	struct ByteArray* wave;
	CoreMixer_type type;
};

void CoreMixer_defaults(struct CoreMixer* self);
void CoreMixer_ctor(struct CoreMixer* self);
struct CoreMixer* CoreMixer_new(void);
void CoreMixer_initialize(struct CoreMixer* self);
int CoreMixer_get_complete(struct CoreMixer* self);
void CoreMixer_set_complete(struct CoreMixer* self, int value);
int CoreMixer_get_bufferSize(struct CoreMixer* self);
void CoreMixer_set_bufferSize(struct CoreMixer* self, int value);
struct ByteArray* CoreMixer_waveform(struct CoreMixer* self);

/* stub */
void CoreMixer_reset(struct CoreMixer* self);
/* stub */
void CoreMixer_fast(struct CoreMixer* self, struct SampleDataEvent *e);
/* stub */
void CoreMixer_accurate(struct CoreMixer* self, struct SampleDataEvent *e);


//RcB: DEP "CoreMixer.c"
#endif 
