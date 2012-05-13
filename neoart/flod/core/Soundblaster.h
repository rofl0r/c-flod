#ifndef SOUNDBLASTER_H
#define SOUNDBLASTER_H

#include "CoreMixer.h"
#include "SBChannel.h"

// extends CoreMixer
struct Soundblaster  {
	struct CoreMixer super;
	//channels : Vector.<SBChannel>;
	struct SBChannel* channels;
};

void Soundblaster_defaults(struct Soundblaster* self);

void Soundblaster_ctor(struct Soundblaster* self);

struct Soundblaster* Soundblaster_new(void);

void Soundblaster_setup(struct Soundblaster* self, int len);
void Soundblaster_initialize(struct Soundblaster* self);
void Soundblaster_fast(struct Soundblaster* self, struct SampleDataEvent* e);
void Soundblaster_accurate(struct Soundblaster* self, struct SampleDataEvent* e);

#endif
