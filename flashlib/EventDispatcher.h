#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#include "Event.h"

struct EventDispatcher {
	//void (*dispatchers[EVENT_MAX])(struct EventDispatcher* self, struct Event *e);
};

typedef void (*EventHandlerFunc)(struct EventDispatcher* self, struct Event *e);

void EventDispatcher_defaults(struct EventDispatcher* self);
void EventDispatcher_ctor(struct EventDispatcher* self);
struct EventDispatcher* EventDispatcher_new(void);

void EventDispatcher_removeEvents(struct EventDispatcher* self);
void EventDispatcher_dispatchEvent(struct EventDispatcher* self, struct Event* e);

void EventDispatcher_addEventListener(struct EventDispatcher* self, enum EventTypes evtType, EventHandlerFunc handler);
void EventDispatcher_removeEventListener(struct EventDispatcher* self, enum EventTypes evtType, EventHandlerFunc handler);

void EventDispatcher_event_loop(void);

#endif
