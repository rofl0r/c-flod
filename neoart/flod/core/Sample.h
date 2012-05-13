#ifndef SAMPLE_H
#define SAMPLE_H

#include "../../../flashlib/Common.h"

struct Sample {
      Number l;
      Number r;
      struct Sample *next;
};

void Sample_defaults(struct Sample* self);

void Sample_ctor(struct Sample* self);

struct Sample* Sample_new(void);

#endif

