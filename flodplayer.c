#include <time.h>

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

enum PlayerType {
	P_S_FT2 = 0,
	P_A_PT,
	P_A_DW,
	P_A_FC,
	P_A_ST,
	P_MAX
};

enum HardwareType {
	HT_SB,
	HT_AMIGA,
	HT_MAX,
};

static const char player_hardware[] = {
	[P_S_FT2] = HT_SB,
	[P_A_PT]  = HT_AMIGA,
	[P_A_DW]  = HT_AMIGA,
	[P_A_FC]  = HT_AMIGA,
	[P_A_ST]  = HT_AMIGA,
};

typedef void (*player_ctor_func) (struct CorePlayer*, struct CoreMixer*);
typedef void (*hardware_ctor_func) (struct CoreMixer*);

static const player_ctor_func player_ctors[] = {
	[P_A_PT]  = (player_ctor_func) PTPlayer_ctor,
	[P_A_ST]  = (player_ctor_func) STPlayer_ctor,
	[P_A_FC]  = (player_ctor_func) FCPlayer_ctor,
	[P_A_DW]  = (player_ctor_func) DWPlayer_ctor,
	[P_S_FT2] = (player_ctor_func) F2Player_ctor,
};

static const hardware_ctor_func hardware_ctors[] = {
	[HT_SB]    = (hardware_ctor_func) Soundblaster_ctor,
	[HT_AMIGA] = (hardware_ctor_func) Amiga_ctor,
};

enum BackendType {
	BE_WAVE,
	BE_AO,
	BE_MAX,
};

typedef int (*backend_write_func) (struct Backend *, void*, size_t);
typedef int (*backend_close_func) (struct Backend *);
typedef int (*backend_init_func)  (struct Backend *, void*);

static const struct BackendInfo {
	const char *name;
	backend_init_func  init_func;
	backend_write_func write_func;
	backend_close_func close_func;
} backend_info[] = {
	[BE_WAVE] = {
		.name = "WaveWriter",
		.init_func  = (backend_init_func)  WaveWriter_init,
		.write_func = (backend_write_func) WaveWriter_write,
		.close_func = (backend_close_func) WaveWriter_close,
	},
	[BE_AO] = {
		.name = "AoWriter",
		.init_func  = (backend_init_func)  AoWriter_init,
		.write_func = (backend_write_func) AoWriter_write,
		.close_func = (backend_close_func) AoWriter_close,
	},
};

int main(int argc, char** argv) {
	int startarg;
	enum BackendType backend_type = BE_AO;
	
	for(startarg = 1; startarg < argc; startarg++) {
		if(argv[startarg][0] == '-' && argv[startarg][1] == 'w')
			backend_type = BE_WAVE;
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
	

	unsigned i;
	enum HardwareType current_hw = HT_MAX;
	
	for(i = 0; i < P_MAX; i++) {
		if(current_hw != player_hardware[i]) {
			current_hw = player_hardware[i];
			hardware_ctors[current_hw](&hardware.core);
		}
		player_ctors[i](&player.core, &hardware.core);
		if(ByteArray_get_length(&stream) > player.core.min_filesize) {
			CorePlayer_load(&player.core, &stream);
			if (player.core.version) goto play;
		}
	}
	
	printf("couldn't find a player for %s\n", argv[startarg]);
	return 1;

	union {
		struct Backend backend;
		struct WaveWriter ww;
		struct AoWriter ao;
	} writer;

play:
	if(!backend_info[backend_type].init_func(&writer.backend, "foo.wav")) {
		perror(backend_info[backend_type].name);
		return 1;
	}
	
	unsigned char wave_buffer[SOUNDCHANNEL_BUFFER_MAX]; 
	// FIXME SOUNDCHANNEL_BUFFER_MAX is currently needed, because the CoreMixer descendants will 
	// misbehave if the stream buffer size is not COREMIXER_MAX_BUFFER * 2 * sizeof(float)
	struct ByteArray wave;
	ByteArray_ctor(&wave);
	wave.endian = BAE_LITTLE;
	
	ByteArray_open_mem(&wave, wave_buffer, sizeof(wave_buffer));
	
	hardware.core.wave = &wave;
	
	player.core.initialize(&player.core);
	
#define MAX_PLAYTIME (60 * 5)
	time_t stoptime = time(NULL) + MAX_PLAYTIME;
	
	while(!CoreMixer_get_complete(&hardware.core)) {
		hardware.core.accurate(&hardware.core, NULL);
		if(wave.pos) {
			backend_info[backend_type].write_func(&writer.backend, wave_buffer, wave.pos);
			wave.pos = 0;
		} else {
			printf("wave pos is 0\n");
			break;
		}
		if(time(NULL) > stoptime) {
			printf("hit timeout\n");
			break;
		}
	}
	
	backend_info[backend_type].close_func(&writer.backend);
	
	printf("finished playing %s\n", argv[startarg]);
	
	return 0;
}

