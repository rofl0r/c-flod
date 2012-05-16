#include "SoundChannel.h"
#include "../neoart/flod/flod_internal.h"
#include "../neoart/flod/flod.h"
#include "../neoart/flod/core/CorePlayer.h"
#include "../neoart/flod/core/Amiga.h"

void SoundChannel_defaults(struct SoundChannel* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

static void send_idle_event(struct SoundChannel* self) {
	EventDispatcher_dispatchEvent((struct EventDispatcher*) self, (struct Event*) &self->idle_event);
}

static void send_need_data_event(struct SoundChannel* self) {
	assert(self->param != NULL);
	EventDispatcher_dispatchEvent((struct EventDispatcher*) self->param, (struct Event*) &self->need_data_event);
}

void SoundChannel_ctor(struct SoundChannel* self) {
	CLASS_CTOR_DEF(SoundChannel);
	// original constructor code goes here
	self->data = ByteArray_new();
	ByteArray_open_mem(self->data, self->buffer, SOUNDCHANNEL_BUFFER_MAX);
	//self->need_data_event = SampleDataEvent_new();
	memset(&self->need_data_event, 0, sizeof(self->need_data_event));
	self->need_data_event.super.type = EVENT_SAMPLE_DATA;
	self->need_data_event.data = self->data;
	
	self->idle_event.type = EVENT_IDLE;
	
	self->playing = 1;
	
	EventDispatcher_addEventListener((struct EventDispatcher*) self, EVENT_IDLE, SoundChannel_idle);
	send_idle_event(self);
	self->fd = open("test.wav", O_WRONLY | O_CREAT | O_TRUNC, 0666);
}

struct SoundChannel* SoundChannel_new(void) {
	CLASS_NEW_BODY(SoundChannel);
}

off_t SoundChannel_get_position(struct SoundChannel *self) {
	PFUNC();
	return self->data->pos;
}

void SoundChannel_stop(struct SoundChannel *self) {
	PFUNC();
	self->playing = 0;
}

static int finish_him(struct SoundChannel *self) {
	close(self->fd);
	ByteArray_dump_to_file(((struct Amiga*) self->param)->super.wave, "foo.wav");
	//CorePlayer_save_record(self->param, "foo.wav");
	exit(1);
}

void SoundChannel_idle(struct SoundChannel *self, struct Event* e)  {
	static int counter = 0;
	// FIXME: when the tune is finished, self playing is still true
	// but self->data->pos is 0, which could also be the case at start
	// find out how to detect the song end
	// TODO find out how to loop a whittaker tune, as in the game dogs of war.
	if(self->playing) {
		printf("stream pos %d\n", self->data->pos);
		counter++;
		if(counter > 3000) {
			finish_him(self);
		}
		if(self->data->pos) {
			write(self->fd, self->data->start_addr, self->data->pos);
			self->data->pos = 0;

		}
		send_need_data_event(self);
		send_idle_event(self);
	} else if(((struct Amiga*) self->param)->super.wave->pos) {
		finish_him(self);
	}
}

void SoundChannel_setParam(struct SoundChannel *self, void* param) {
	self->param = param;
}


