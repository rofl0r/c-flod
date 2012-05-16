#ifndef SOUNDCHANNEL_H
#define SOUNDCHANNEL_H

#include "../neoart/flod/flod.h"
#include "../neoart/flod/core/CoreMixerMaxBuffer.h"
#include "EventDispatcher.h"

#define SOUNDCHANNEL_BUFFER_MAX (COREMIXER_MAX_BUFFER * (sizeof(float)) * 2)

/*
flash type
inheritance
Object
	-> EventDispatcher
		-> SoundChannel
*/
struct SoundChannel {
	struct EventDispatcher super;
	
	struct ByteArray* data;
	struct SampleDataEvent need_data_event;
	struct Event idle_event;
	int playing;
	unsigned char buffer[SOUNDCHANNEL_BUFFER_MAX];
	void* param; //used to store the hardware pointer (e.g. Amiga / Soundblaster)
	
	int fd;
};



void SoundChannel_defaults(struct SoundChannel* self);
void SoundChannel_ctor(struct SoundChannel* self);
struct SoundChannel* SoundChannel_new(void);

off_t SoundChannel_get_position(struct SoundChannel *self);
void SoundChannel_stop(struct SoundChannel *self);
void SoundChannel_idle(struct SoundChannel *self, struct Event* e);
void SoundChannel_setParam(struct SoundChannel *self, void* param);

#endif