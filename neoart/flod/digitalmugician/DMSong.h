#ifndef DMSONG_H
#define DMSONG_H

#include "../core/AmigaStep.h"

#define DMSONG_MAX_TRACKS 16

/*
inheritance
object
	-> DMSong
*/
struct DMSong {
	char *title;
	int speed;
	int length;
	int loop;
	int loopStep;
	//tracks   : Vector.<AmigaStep>;
	struct AmigaStep tracks[DMSONG_MAX_TRACKS];
};

void DMSong_defaults(struct DMSong* self);
void DMSong_ctor(struct DMSong* self);
struct DMSong* DMSong_new(void);

#endif
