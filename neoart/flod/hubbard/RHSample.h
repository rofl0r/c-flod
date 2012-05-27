#ifndef RHSAMPLE_H
#define RHSAMPLE_H

#include "../core/AmigaSample.h"

#define RHSAMPLE_MAX_WAVE 4

struct RHSample {
	struct AmigaSample super;
	int relative;
	int divider;
	int vibrato;
	int hiPos;
	int loPos;
	//wave     : Vector.<int>;
	int wave[RHSAMPLE_MAX_WAVE];
};

void RHSample_defaults(struct RHSample* self);
void RHSample_ctor(struct RHSample* self);
struct RHSample* RHSample_new(void);

#endif
