#define _GNU_SOURCE
#include <time.h>
#include <errno.h>

#include "EventDispatcher.h"
#include "../neoart/flod/flod_internal.h"
#include "../neoart/flod/flod.h"


static void (*dispatchers[EVENT_MAX])(struct EventDispatcher* self, struct Event *e);
#define MAX_EVENTS 4

static unsigned int evtCounter = 0;

typedef struct {
	struct Event* evtptr;
	struct EventDispatcher* owner;
} evtQueueElem;

static evtQueueElem evtQueue[MAX_EVENTS];

void EventDispatcher_defaults(struct EventDispatcher* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void EventDispatcher_ctor(struct EventDispatcher* self) {
	CLASS_CTOR_DEF(EventDispatcher);
	// original constructor code goes here
}

struct EventDispatcher* EventDispatcher_new(void) {
	CLASS_NEW_BODY(EventDispatcher);
}

void EventDispatcher_removeEvents(struct EventDispatcher* self) {
	PFUNC();
}

static evtQueueElem *get_free_slot(void) {
	evtQueueElem *slot = NULL;
	unsigned i;
	for (i = 0; i < MAX_EVENTS; i++) {
		slot = &evtQueue[i];
		if(!slot->evtptr) return slot;
	}
	return NULL;
}

void EventDispatcher_dispatchEvent(struct EventDispatcher* self, struct Event* e) {
	PFUNC();
	printf("eventumber :%d, have? %d\n", e->type, !!dispatchers[e->type]);
	evtQueueElem* slot = get_free_slot();
	/* warning: if more than MAX_EVENTS events queued,
	the first element gets overwritten, and queue could be processed in a different order
	*/
	if(!slot) {
		INT3;
		printf("more than MAX_EVENTS events queued\n");
		exit(1);
	}
	evtCounter++;
	slot->evtptr = e;
	slot->owner = self;
}

void EventDispatcher_addEventListener(struct EventDispatcher* self, enum EventTypes evtType, EventHandlerFunc handler) {
	PFUNC();
	dispatchers[evtType] = handler;
	printf("eventumber :%d\n", evtType);
}

void EventDispatcher_removeEventListener(struct EventDispatcher* self, enum EventTypes evtType, EventHandlerFunc handler) {
	PFUNC();
	dispatchers[evtType] = NULL;
}

static int msleep(long millisecs) {
	struct timespec req, rem;
	req.tv_sec = millisecs / 1000;
	req.tv_nsec = (millisecs % 1000) * 1000 * 1000;
	int ret;
	while((ret = nanosleep(&req, &rem)) == -1 && errno == EINTR) req = rem;
	return ret;	
}

void EventDispatcher_event_loop(void) {
	while(1) {
		unsigned int dispatcherlist_empty = 1;
		unsigned int i;
		for(i = 0; i < EVENT_MAX; i++) {
			if(dispatchers[i]) {
				dispatcherlist_empty = 0;
				break;
			}
		}
		if(dispatcherlist_empty) break;

again:
		for(i = 0; evtCounter && i < MAX_EVENTS; i++) {
			evtQueueElem *slot = &evtQueue[i];
			evtQueueElem item = *slot;;
			if(item.evtptr && dispatchers[item.evtptr->type]) {
				printf("executing evt %d\n", item.evtptr->type);
				slot->evtptr = NULL;
				slot->owner = NULL;
				evtCounter--;
				dispatchers[item.evtptr->type](item.owner, item.evtptr);
				goto again;
			}
		}
		msleep(2);
	}
}






