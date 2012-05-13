#ifndef COREMIXER_H
#define COREMIXER_H

#include "CorePlayer.h"
#include "Sample.h"
#include "../flod.h"

struct CoreMixer {
	struct CorePlayer* player;
	int samplesTick;
	// buffer      : Vector.<Sample>,
	struct Sample* buffer; //Vector
	int samplesLeft;
	int remains;
	int completed;
	struct ByteArray* wave;
};

void CoreMixer_defaults(struct CoreMixer* self);
void CoreMixer_ctor(struct CoreMixer* self);
struct CoreMixer* CoreMixer_new(void);
void CoreMixer_initialize(struct CoreMixer* self);
int CoreMixer_get_complete(struct CoreMixer* self);
void CoreMixer_set_complete(struct CoreMixer* self, int value);
void CoreMixer_reset(struct CoreMixer* self);
void CoreMixer_fast(struct CoreMixer* self, struct SampleDataEvent *e);
void CoreMixer_accurate(struct CoreMixer* self, struct SampleDataEvent *e);
int CoreMixer_get_bufferSize(struct CoreMixer* self);
void CoreMixer_set_bufferSize(struct CoreMixer* self, int value);
struct ByteArray* CoreMixer_waveform(struct CoreMixer* self);

//RcB: DEP "CoreMixer.c"
#endif 
