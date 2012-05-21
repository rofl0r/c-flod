#ifndef F2VOICE_H
#define F2VOICE_H

#include "../core/SBChannel.h"
#include "F2Instrument.h"
#include "F2Sample.h"
#include "F2Envelope.h"

/*
inheritance
object
	-> F2Voice
*/
struct F2Voice {
	int index;
	struct F2Voice *next;
	int flags;
	int delay;
	struct SBChannel *channel;
	int patternLoop;
	int patternLoopRow;
	struct F2Instrument *playing;
	int note;
	int keyoff;
	int period;
	int finetune;
	int arpDelta;
	int vibDelta;
	struct F2Instrument *instrument;
	int autoVibratoPos;
	int autoSweep;
	int autoSweepPos;
	struct F2Sample *sample;
	int sampleOffset;
	int volume;
	int volEnabled;
	struct F2Envelope *volEnvelope;
	int volDelta;
	int volSlide;
	int volSlideMaster;
	int fineSlideU;
	int fineSlideD;
	int fadeEnabled;
	int fadeDelta;
	int fadeVolume;
	int panning;
	int panEnabled;
	struct F2Envelope *panEnvelope;
	int panSlide;
	int portaU;
	int portaD;
	int finePortaU;
	int finePortaD;
	int xtraPortaU;
	int xtraPortaD;
	int portaPeriod;
	int portaSpeed;
	int glissando;
	int glissPeriod;
	int vibratoPos;
	int vibratoSpeed;
	int vibratoDepth;
	int vibratoReset;
	int tremoloPos;
	int tremoloSpeed;
	int tremoloDepth;
	int waveControl;
	int tremorPos;
	int tremorOn;
	int tremorOff;
	int tremorVolume;
	int retrigx;
	int retrigy;
};

void F2Voice_defaults(struct F2Voice* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void F2Voice_ctor(struct F2Voice* self, int index) {
	CLASS_CTOR_DEF(F2Voice);
	// original constructor code goes here
}

struct F2Voice* F2Voice_new(int index) {
	CLASS_NEW_BODY(F2Voice, index);
}


#endif
