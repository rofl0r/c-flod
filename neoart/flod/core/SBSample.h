#ifndef SBSAMPLE_H
#define SBSAMPLE_H

struct SBSample {
	char* name; //      : String = "",
	int bits; //      : int = 8,
	int volume;
	int length;
	//data      : Vector.<Number>,
	Number* data;
	int loopMode;
	int loopStart;
	int loopLen;
};

void SBSample_defaults(struct SBSample* self);

void SBSample_ctor(struct SBSample* self);

struct SBSample* SBSample_new(void);

void SBSample_store(struct SBSample* self, struct ByteArray* stream);

#endif
