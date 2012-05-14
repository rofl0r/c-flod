#ifndef COREPLAYER_H
#define COREPLAYER_H

#include "../flod.h"
#include "CoreMixer.h"
#include "../../../flashlib/Sound.h"
#include "../../../flashlib/SoundChannel.h"
#include "../../../flashlib/EventDispatcher.h"

struct CorePlayer {
	//extends EventDispatcher
	struct EventDispatcher super;
	enum Encoding encoding;
	int quality;
	int record;
	int playSong;
	int lastSong;
	int version;
	int variant;
	char* title;
	int channels;
	int loopSong;
	int speed;
	int tempo;
	//protected
	struct CoreMixer* hardware;
	struct Sound* sound;
	struct SoundChannel* soundChan;
	Number soundPos;
	char* endian;
	int tick;
};

void CorePlayer_defaults(struct CorePlayer* self);
void CorePlayer_ctor(struct CorePlayer* self, struct CoreMixer *hardware);
struct CorePlayer* CorePlayer_new(struct CoreMixer *hardware);
void CorePlayer_set_force(struct CorePlayer* self, int value);
void CorePlayer_set_ntsc(struct CorePlayer* self, int value);
void CorePlayer_set_stereo(struct CorePlayer* self, Number value);
void CorePlayer_set_volume(struct CorePlayer* self, Number value);
struct ByteArray *CorePlayer_get_waveform(struct CorePlayer* self);
void CorePlayer_toggle(struct CorePlayer* self, int index);
int CorePlayer_load(struct CorePlayer* self, struct ByteArray *stream);
int CorePlayer_play(struct CorePlayer* self, struct Sound *processor);
void CorePlayer_pause(struct CorePlayer* self);
void CorePlayer_stop(struct CorePlayer* self);
void CorePlayer_process(struct CorePlayer* self);
void CorePlayer_fast(struct CorePlayer* self);
void CorePlayer_accurate(struct CorePlayer* self);
void CorePlayer_setup(struct CorePlayer* self);
void CorePlayer_initialize(struct CorePlayer* self);
void CorePlayer_reset(struct CorePlayer* self);
void CorePlayer_loader(struct CorePlayer* self, struct ByteArray *stream);
void CorePlayer_completeHandler(struct CorePlayer* self, struct Event *e);
void CorePlayer_removeEvents(struct CorePlayer* self);


#endif
