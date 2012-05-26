#ifndef S2INSTRUMENT_H
#define S2INSTRUMENT_H

struct S2Instrument {
	int wave;
	int waveLen;
	int waveDelay;
	int waveSpeed;
	int arpeggio;
	int arpeggioLen;
	int arpeggioDelay;
	int arpeggioSpeed;
	int vibrato;
	int vibratoLen;
	int vibratoDelay;
	int vibratoSpeed;
	int pitchBend;
	int pitchBendDelay;
	int attackMax;
	int attackSpeed;
	int decayMin;
	int decaySpeed;
	int sustain;
	int releaseMin;
	int releaseSpeed;
};

void S2Instrument_defaults(struct S2Instrument* self);
void S2Instrument_ctor(struct S2Instrument* self);
struct S2Instrument* S2Instrument_new(void);

#endif
