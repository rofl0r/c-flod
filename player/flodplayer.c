#include <time.h>

#include "../backends/wavewriter.h"
#include "../backends/aowriter.h"

#include "../flashlib/ByteArray.h"

#include "../neoart/flod/core/CorePlayer.h"
#include "../neoart/flod/fasttracker/F2Player.h"
#include "../neoart/flod/trackers/PTPlayer.h"
#include "../neoart/flod/trackers/STPlayer.h"
#include "../neoart/flod/whittaker/DWPlayer.h"
#include "../neoart/flod/futurecomposer/FCPlayer.h"

#include "keyboard.h"

enum KeyboardCommand {
	KC_NONE,
	KC_PAUSE,
	KC_QUIT,
	KC_NEXT,
	KC_SKIP,
};

int check_keyboard(void) {
	if(kbhit()) {
		switch(readch()) {
			case 0x03: // CTRL-C
			case 0x04: // CTRL-D
			case 0x1a: // CTRL-Z
			case 0x1b: // ESC
			case 'q':
			case 'x':
				return KC_QUIT;
			case 'p':
				return KC_PAUSE;
			case '\t': case '\r': case '\n':
				return KC_NEXT;
			case '.':
				return KC_SKIP;
			default:
				break;
		}
	}
	return KC_NONE;
}

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

static const char *player_name[] = {
	[P_S_FT2] = "FastTracker 2",
	[P_A_PT]  = "ProTracker",
	[P_A_DW]  = "David Whittaker",
	[P_A_FC]  = "Future Composer",
	[P_A_ST]  = "SoundTracker",
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


static union {
	struct CorePlayer core;
	struct F2Player f2;
	struct PTPlayer pt;
	struct STPlayer st;
	struct FCPlayer fc;
	struct DWPlayer dw;
} player;

static union {
	struct CoreMixer core;
	struct Amiga amiga;
	struct Soundblaster soundblaster;
} hardware;

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
	
	printf("opening %s\n", argv[startarg]);
	
	struct ByteArray stream;
	ByteArray_ctor(&stream);
	
	if(!ByteArray_open_file(&stream, argv[startarg])) {
		perror("couldnt open file");
		return 1;
	}
	
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
			if (player.core.version) {
				printf("::: using %s player :::\n", player_name[i]);
				goto play;
			}
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
	
	unsigned char wave_buffer[COREMIXER_MAX_BUFFER * 2 * sizeof(float)]; 
	// FIXME SOUNDCHANNEL_BUFFER_MAX is currently needed, because the CoreMixer descendants will 
	// misbehave if the stream buffer size is not COREMIXER_MAX_BUFFER * 2 * sizeof(float)
	struct ByteArray wave;
	ByteArray_ctor(&wave);
	wave.endian = BAE_LITTLE;
	
	ByteArray_open_mem(&wave, wave_buffer, sizeof(wave_buffer));
	
	hardware.core.wave = &wave;
	
	player.core.initialize(&player.core);
	
#define MAX_PLAYTIME (60 * 5)
	const unsigned bytespersec = 44100 * 2 * (16 / 8);
	const unsigned max_bytes = bytespersec * MAX_PLAYTIME;
	unsigned bytes_played = 0;
	int paused = 0, skip = 0;
	
	init_keyboard();
	
	while(!CoreMixer_get_complete(&hardware.core)) {
		hardware.core.accurate(&hardware.core);
		if(wave.pos) {
			if(!skip)
				backend_info[backend_type].write_func(&writer.backend, wave_buffer, wave.pos);
			else
				skip--;
			bytes_played += wave.pos;
			wave.pos = 0;
		} else {
			printf("wave pos is 0\n");
			break;
		}
		if(bytes_played >= max_bytes) {
			printf("hit timeout\n");
			break;
		}
		enum KeyboardCommand kc;
ck:
		if((kc = check_keyboard()) == KC_QUIT) break;
		else if(kc == KC_PAUSE) paused = !paused;
		else if(kc == KC_SKIP) skip += 10;
		if(paused) {
			sleep(1);
			goto ck;
		}
	}
	
	backend_info[backend_type].close_func(&writer.backend);
	
	printf("finished playing %s\n", argv[startarg]);
	
	close_keyboard();
	
	return 0;
}

