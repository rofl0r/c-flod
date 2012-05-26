#ifndef S1PLAYER_H
#define S1PLAYER_H

#include "../core/AmigaPlayer.h"
#include "../core/AmigaStep.h"
#include "../core/Amiga.h"
#include "SMRow.h"
#include "S1Sample.h"
#include "S1Voice.h"

enum S1PlayerVersion {
	SIDMON_0FFA = 0x0ffa,
	SIDMON_1170 = 0x1170,
	SIDMON_11C6 = 0x11c6,
	SIDMON_11DC = 0x11dc,
	SIDMON_11E0 = 0x11e0,
	SIDMON_125A = 0x125a,
	SIDMON_1444 = 0x1444,
};

#define S1PLAYER_TRACKSPTR_MAX 4
#define S1PLAYER_PATTERNSPTR_MAX 4
#define S1PLAYER_WAVELISTS_MAX 4

#define S1PLAYER_TRACKS_MAX 4
#define S1PLAYER_PATTERNS_MAX 4
#define S1PLAYER_SAMPLES_MAX 4
#define S1PLAYER_VOICES_MAX 4


struct S1Player {
	struct AmigaPlayer super;
	//tracksPtr   : Vector.<int>,
	int tracksPtr[S1PLAYER_TRACKSPTR_MAX];
	//patternsPtr : Vector.<int>,
	int patternsPtr[S1PLAYER_PATTERNSPTR_MAX];
	//waveLists   : Vector.<int>,
	int waveLists[S1PLAYER_WAVELISTS_MAX];
	
	//tracks      : Vector.<AmigaStep>,
	struct AmigaStep tracks[S1PLAYER_TRACKS_MAX];
	//patterns    : Vector.<SMRow>,
	struct SMRow patterns[S1PLAYER_PATTERNS_MAX];
	//samples     : Vector.<S1Sample>,
	struct S1Sample samples[S1PLAYER_SAMPLES_MAX];
	//voices      : Vector.<S1Voice>,
	struct S1Voice voices[S1PLAYER_VOICES_MAX];
	
	int speedDef;
	int trackLen;
	int patternDef;
	int mix1Speed;
	int mix2Speed;
	int mix1Dest;
	int mix2Dest;
	int mix1Source1;
	int mix1Source2;
	int mix2Source1;
	int mix2Source2;
	int doFilter;
	int doReset;
	int trackPos;
	int trackEnd;
	int patternPos;
	int patternEnd;
	int patternLen;
	int mix1Ctr;
	int mix2Ctr;
	int mix1Pos;
	int mix2Pos;
	int audPtr;
	int audLen;
	int audPer;
	int audVol;
};

#endif
