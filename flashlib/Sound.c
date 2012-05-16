#include "Sound.h"
#include "../neoart/flod/flod_internal.h"

void Sound_defaults(struct Sound* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void Sound_ctor(struct Sound* self) {
	CLASS_CTOR_DEF(Sound);
	// original constructor code goes here
}

struct Sound* Sound_new(void) {
	CLASS_NEW_BODY(Sound);
}

struct SoundChannel* Sound_play(struct Sound* self, Number soundpos, void* hardware) {
	PFUNC();
	struct SoundChannel* ret = SoundChannel_new();
	SoundChannel_setParam(ret, hardware);
	return ret;
}
