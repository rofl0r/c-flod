#ifndef SOUND_H
#define SOUND_H
#include "SoundChannel.h"
#include "Common.h"
// flash.media.Sounds
/*
flash type
inheritance
Object
	-> EventDispatcher
		-> Sound
*/
struct Sound {
	struct EventDispatcher super;
};

void Sound_defaults(struct Sound* self);
void Sound_ctor(struct Sound* self);
struct Sound* Sound_new(void);

struct SoundChannel* Sound_play(struct Sound* self, Number soundpos, void* hardware);

#endif
