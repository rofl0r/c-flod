#ifndef EVENT_H
#define EVENT_H

enum EventTypes {
	EVENT_SOUND_COMPLETE = 0,
	EVENT_SAMPLE_DATA,
	EVENT_IDLE,
	EVENT_MAX,
};

/*
flash type
inheritance
object
	-> Event 
*/
struct Event {
	enum EventTypes type;
};

void Event_defaults(struct Event* self);
void Event_ctor(struct Event* self, enum EventTypes evtType);
struct Event* Event_new(enum EventTypes evtType);

#endif
