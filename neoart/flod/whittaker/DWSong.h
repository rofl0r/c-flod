#ifndef DWSONG_H
#define DWSONG_H

struct DWSong {
	int speed;
	int delay;
	//tracks : Vector.<int>;
	int *tracks;
};

void DWSong_defaults(struct DWSong* self);
void DWSong_ctor(struct DWSong* self);
struct DWSong* DWSong_new(void);

#endif
