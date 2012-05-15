#ifndef SBSAMPLE_H
#define SBSAMPLE_H

#define SBSAMPLE_MAX_DATA 128

#include "../../../flashlib/ByteArray.h"

/*
inheritance
object
	-> SBSample
*/
struct SBSample {
	char* name; //      : String = "",
	int bits; //      : int = 8,
	int volume;
	int length;
	//data      : Vector.<Number>,
	//Number* data;
	Number data[SBSAMPLE_MAX_DATA];
	int loopMode;
	int loopStart;
	int loopLen;
};

void SBSample_defaults(struct SBSample* self);
void SBSample_ctor(struct SBSample* self);
struct SBSample* SBSample_new(void);

void SBSample_store(struct SBSample* self, struct ByteArray* stream);

#endif
