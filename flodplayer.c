#include "backends/wavewriter.h"
#include "backends/aowriter.h"

#include "flashlib/ByteArray.h"
#include "flashlib/SoundChannel.h"

#include "neoart/flod/core/CorePlayer.h"
#include "neoart/flod/fasttracker/F2Player.h"
#include "neoart/flod/trackers/PTPlayer.h"
#include "neoart/flod/trackers/STPlayer.h"
#include "neoart/flod/whittaker/DWPlayer.h"
#include "neoart/flod/futurecomposer/FCPlayer.h"

enum {
	AP_PT = 0,
	AP_DW,
	AP_FC,
	AP_ST,
	AP_MAX
} AmigaPlayers;

enum {
	SP_FT2,
	SP_MAX
} SoundblasterPlayers;

typedef void (*player_ctor_func) (struct CorePlayer*, struct CoreMixer*);
typedef int (*backend_write_func) (struct Backend *, void*, size_t);
typedef int (*backend_close_func) (struct Backend *);

int main(int argc, char** argv) {
	int startarg;
	int wave_backend = 0;
	
	for(startarg = 1; startarg < argc; startarg++) {
		if(argv[startarg][0] == '-' && argv[startarg][1] == 'w')
			wave_backend = 1;
		else
			break;
	}
	if(startarg == argc) {
		printf("usage: %s [-w] filename.mod\n"
		       "where -w means write output to foo.wav instead of audio device\n", argv[0]);
		return 1;
	}
	
	struct ByteArray stream;
	ByteArray_ctor(&stream);	
	
	if(!ByteArray_open_file(&stream, argv[startarg])) {
		perror("couldnt open file");
		return 1;
	}
	
	union {
		struct CorePlayer core;
		struct F2Player f2;
		struct PTPlayer pt;
		struct STPlayer st;
		struct FCPlayer fc;
		struct DWPlayer dw;
	} player;
	
	union {
		struct CoreMixer core;
		struct Amiga amiga;
		struct Soundblaster soundblaster;
	} hardware;
	
	player_ctor_func Amiga_ctors[] = {
		[AP_PT] = (player_ctor_func) PTPlayer_ctor,
		[AP_ST] = (player_ctor_func) STPlayer_ctor,
		[AP_FC] = (player_ctor_func) FCPlayer_ctor,
		[AP_DW] = (player_ctor_func) DWPlayer_ctor,
	};
	
	player_ctor_func Soundblaster_ctors[] = {
		[SP_FT2] = (player_ctor_func) F2Player_ctor,
	};
	
	unsigned i;
	
	Soundblaster_ctor(&hardware.soundblaster);
	for(i = 0; i < SP_MAX; i++) {
		Soundblaster_ctors[i](&player.core, &hardware.core);
		CorePlayer_load(&player.core, &stream);
		if (player.core.version) goto play;
	}
	
	Amiga_ctor(&hardware.amiga);
	for(i = 0; i < AP_MAX; i++) {
		Amiga_ctors[i](&player.core, &hardware.core);
		CorePlayer_load(&player.core, &stream);
		if (player.core.version) goto play;
	}
	
	printf("couldn't find a player for %s\n", argv[1]);
	return 1;

	backend_write_func backend_write;
	backend_close_func backend_close;
	
	union {
		struct Backend backend;
		struct WaveWriter ww;
		struct AoWriter ao;
	} writer;

play:
	
	if(wave_backend) {
		backend_write = WaveWriter_write;
		backend_close = WaveWriter_close;
		if(!WaveWriter_init(&writer.ww, "foo.wav")) {
			perror("wavewriter");
			return 1;
		}
	} else {
		backend_write = AoWriter_write;
		backend_close = AoWriter_close;
		if(!AoWriter_init(&writer.ao)) {
			perror("aowriter");
			return 1;
		}
	}
	
	unsigned char wave_buffer[SOUNDCHANNEL_BUFFER_MAX];
	struct ByteArray wave;
	ByteArray_ctor(&wave);
	wave.endian = BAE_LITTLE;
	
	ByteArray_open_mem(&wave, wave_buffer, sizeof(wave_buffer));
	
	hardware.core.wave = &wave;
	
	player.core.initialize(&player.core);
	
	
	
	while(!CoreMixer_get_complete(&hardware.core)) {
		hardware.core.accurate(&hardware.core, NULL);
		if(wave.pos) {
			backend_write(&writer.backend, wave_buffer, wave.pos);
			wave.pos = 0;
		} else {
			printf("wave pos is 0\n");
			break;
		}
	}
	
	backend_close(&writer.backend);
	
	return 0;
}

