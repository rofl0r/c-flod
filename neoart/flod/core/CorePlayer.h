#ifndef COREPLAYER_H
#define COREPLAYER_H

#include "../flod.h"

struct CorePlayer {
	//extends EventDispatcher 
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

#endif
