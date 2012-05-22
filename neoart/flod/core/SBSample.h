#ifndef SBSAMPLE_H
#define SBSAMPLE_H

//#define SBSAMPLE_MAX_DATA 220
// FIXME hmm, it seems sample lengths can vary radically.
// probably not the best idea to make them all the same size
#define SBSAMPLE_MAX_DATA (20 * 1024)

#include "../flod.h"
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
	//unsigned vector_count_data;
	int loopMode;
	int loopStart;
	int loopLen;
};

void SBSample_defaults(struct SBSample* self);
void SBSample_ctor(struct SBSample* self);
struct SBSample* SBSample_new(void);

void SBSample_store(struct SBSample* self, struct ByteArray* stream);

#endif
