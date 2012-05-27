#ifndef FETRACK_H
#define FETRACK_H

#define FETRACK_MAX_DATA 4

struct FETrack {
	int track_data[FETRACK_MAX_DATA];
};

void FETrack_defaults(struct FETrack* self);
void FETrack_ctor(struct FETrack* self);
struct FETrack* FETrack_new(void);

#endif
