#include "Event.h"
#include "../neoart/flod/flod_internal.h"

void Event_defaults(struct Event* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void Event_ctor(struct Event* self, enum EventTypes evtType) {
	CLASS_CTOR_DEF(Event);
	// original constructor code goes here
	self->type = evtType;
}

struct Event* Event_new(enum EventTypes evtType) {
	CLASS_NEW_BODY(Event, evtType);
}
