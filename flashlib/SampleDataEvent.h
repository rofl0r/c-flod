#ifndef SAMPLEDATAEVENT_H
#define SAMPLEDATAEVENT_H

#include "ByteArray.h"
#include "Event.h"

struct SampleDataEvent {
	struct Event super;
	struct ByteArray* data;
};

#endif