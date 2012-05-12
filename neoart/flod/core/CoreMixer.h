#ifndef COREMIXER_H
#define COREMIXER_H

#include "CorePlayer.h"
#include "Sample.h"
#include "../flod.h"

struct CoreMixer {
	struct CorePlayer* player;
	int samplesTick;
	struct Sample* buffer; //Vector
	int samplesLeft;
	int remains;
	int completed;
	struct ByteArray* wave;
};
//RcB: DEP "CoreMixer.c"
#endif 
