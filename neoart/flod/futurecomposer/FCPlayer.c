/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.0 - 2012/03/11

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/


#include "FCPlayer.h"
#include "../flod_internal.h"

void FCPlayer_defaults(struct FCPlayer* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

// amiga default value NULL
void FCPlayer_ctor(struct FCPlayer* self, struct Amiga *amiga) {
	CLASS_CTOR_DEF(FCPlayer);
	// original constructor code goes here
	super(amiga);
	PERIODS->fixed = true;
	WAVES->fixed   = true;

	//voices = new Vector.<FCVoice>(4, true);

	voices[0] = new FCVoice(0);
	voices[0].next = voices[1] = new FCVoice(1);
	voices[1].next = voices[2] = new FCVoice(2);
	voices[2].next = voices[3] = new FCVoice(3);	
}

struct FCPlayer* FCPlayer_new(struct Amiga *amiga) {
	CLASS_NEW_BODY(FCPlayer, amiga);
}

//override
void FCPlayer_process(struct FCPlayer* self) {
	int base; 
	struct AmigaChannel *chan;
	int delta;
	int i;
	int info;
	int loopEffect;
	int loopSustain;
	int period;
	struct AmigaSample *sample;
	int temp;
	struct FCVoice *voice = &self->voices[0];

	if (--(self->super.super.tick) == 0) {
		base = self->seqs->position;

		while (voice) {
			chan = voice->channel;

			self->pats->position = voice->pattern + voice->patStep;
			temp = self->pats->readUnsignedByte();

			if (voice->patStep >= 64 || temp == 0x49) {
				if (self->seqs->position == self->length) {
					ByteArray_set_position(self->seqs, 0);
					self->super.amiga->complete = 1;
				}

				voice->patStep = 0;
				voice->pattern = self->seqs->readUnsignedByte() << 6;
				voice->transpose = self->seqs->readByte();
				voice->soundTranspose = self->seqs->readByte();

				ByteArray_set_position(self->pats, voice->pattern);
				temp = self->pats->readUnsignedByte();
			}
			info = self->pats->readUnsignedByte();
			ByteArray_set_position(self->frqs, 0);
			ByteArray_set_position(self->vols, 0);

			if (temp != 0) {
				voice->note = temp & 0x7f;
				voice->pitch = 0;
				voice->portamento = 0;
				voice->enabled = chan->enabled = 0;

				temp = 8 + (((info & 0x3f) + voice->soundTranspose) << 6);
				if (temp >= 0 && temp < self->vols->length) 
					ByteArray_set_position(self->vols, temp);

				voice->volStep = 0;
				voice->volSpeed = voice->volCtr = self->vols->readUnsignedByte();
				voice->volSustain = 0;

				voice->frqPos = 8 + (self->vols->readUnsignedByte() << 6);
				voice->frqStep = 0;
				voice->frqSustain = 0;

				voice->vibratoFlag  = 0;
				voice->vibratoSpeed = self->vols->readUnsignedByte();
				voice->vibratoDepth = voice->vibrato = self->vols->readUnsignedByte();
				voice->vibratoDelay = self->vols->readUnsignedByte();
				voice->volPos = self->vols->position;
			}

			if (info & 0x40) {
				voice->portamento = 0;
			} else if (info & 0x80) {
				voice->portamento = self->pats[int(pats->position + 1)];
				if (self->super.super.version == FUTURECOMP_10) voice->portamento <<= 1;
			}
			voice->patStep += 2;
			voice = voice->next;
		}

		if (self->seqs->position != base) {
			temp = self->seqs->readUnsignedByte();
			if (temp) self->super.super.speed = temp;
		}
		self->super.super.tick = self->super.super.speed;
	}
	voice = &self->voices[0];

	while (voice) {
		chan = voice->channel;

		do {
			loopSustain = 0;

			if (voice->frqSustain) {
				voice->frqSustain--;
				break;
			}
			ByteArray_set_position(self->frqs, voice->frqPos + voice->frqStep);

			do {
				loopEffect = 0;
				if (!self->frqs->bytesAvailable) break;
				info = self->frqs->readUnsignedByte();
				if (info == 0xe1) break;

				if (info == 0xe0) {
					voice->frqStep = self->frqs->readUnsignedByte() & 0x3f;
					ByteArray_set_position(self->frqs, voice->frqPos + voice->frqStep);
					info = self->frqs->readUnsignedByte();
				}

				switch (info) {
					case 0xe2:  //set wave
						chan->enabled  = 0;
						voice->enabled = 1;
						voice->volCtr  = 1;
						voice->volStep = 0;
						// FIXME break forgotten or fallthrough?
					case 0xe4:  //change wave:
						sample = self->samples[self->frqs->readUnsignedByte()];
						if (sample) {
							chan->pointer = sample->pointer;
							chan->length  = sample->length;
						} else {
							voice->enabled = 0;
						}
						voice->sample = sample;
						voice->frqStep += 2;
						break;
					case 0xe9:  //set pack
						temp = 100 + (self->frqs->readUnsignedByte() * 10);
						sample = self->samples[(temp + self->frqs->readUnsignedByte())];

						if (sample) {
							chan->enabled = 0;
							chan->pointer = sample->pointer;
							chan->length  = sample->length;
							voice->enabled = 1;
						}

						voice->sample = sample;
						voice->volCtr = 1;
						voice->volStep = 0;
						voice->frqStep += 3;
						break;
					case 0xe7:  //new sequence
						loopEffect = 1;
						voice->frqPos = 8 + (self->frqs->readUnsignedByte() << 6);
						if (voice->frqPos >= self->frqs->length) voice->frqPos = 0;
						voice->frqStep = 0;
						ByteArray_set_position(self->frqs, voice->frqPos);
						break;
					case 0xea:  //pitch bend
						voice->pitchBendSpeed = self->frqs->readByte();
						voice->pitchBendTime  = self->frqs->readUnsignedByte();
						voice->frqStep += 3;
						break;
					case 0xe8:  //sustain
						loopSustain = 1;
						voice->frqSustain = self->frqs->readUnsignedByte();
						voice->frqStep += 2;
						break;
					case 0xe3:  //new vibrato
						voice->vibratoSpeed = self->frqs->readUnsignedByte();
						voice->vibratoDepth = self->frqs->readUnsignedByte();
						voice->frqStep += 3;
						break;
					default:
						break;
				}

				if (!loopSustain && !loopEffect) {
					ByteArray_set_position(self->frqs, voice->frqPos + voice->frqStep);
					voice->frqTranspose = self->frqs->readByte();
					voice->frqStep++;
				}
			} while (loopEffect);
		} while (loopSustain);

		if (voice->volSustain) {
			voice->volSustain--;
		} else {
			if (voice->volBendTime) {
				voice->volumeBend();
			} else {
				if (--(voice->volCtr) == 0) {
					voice->volCtr = voice->volSpeed;

					do {
						loopEffect = 0;
						ByteArray_set_position(self->vols, voice->volPos + voice->volStep);
						if (!self->vols->bytesAvailable) break;
						info = self->vols->readUnsignedByte();
						if (info == 0xe1) break;

						switch (info) {
							case 0xea: //volume slide
								voice->volBendSpeed = self->vols->readByte();
								voice->volBendTime  = self->vols->readUnsignedByte();
								voice->volStep += 3;
								voice->volumeBend();
								break;
							case 0xe8: //volume sustain
								voice->volSustain = self->vols->readUnsignedByte();
								voice->volStep += 2;
								break;
							case 0xe0: //volume loop
								loopEffect = 1;
								temp = self->vols->readUnsignedByte() & 0x3f;
								voice->volStep = temp - 5;
								break;
							default:
								voice->volume = info;
								voice->volStep++;
								break;
						}
					} while (loopEffect);
				}
			}
		}
		info = voice->frqTranspose;
		if (info >= 0) info += (voice->note + voice->transpose);
		info &= 0x7f;
		period = PERIODS[info];

		if (voice->vibratoDelay) {
			voice->vibratoDelay--;
		} else {
			temp = voice->vibrato;

			if (voice->vibratoFlag) {
				delta = voice->vibratoDepth << 1;
				temp += voice->vibratoSpeed;

				if (temp > delta) {
					temp = delta;
					voice->vibratoFlag = 0;
				}
			} else {
				temp -= voice->vibratoSpeed;

				if (temp < 0) {
					temp = 0;
					voice->vibratoFlag = 1;
				}
			}
			voice->vibrato = temp;
			temp -= voice->vibratoDepth;
			base = (info << 1) + 160;

			while (base < 256) {
				temp <<= 1;
				base += 24;
			}
			period += temp;
		}
		voice->portamentoFlag ^= 1;

		if (voice->portamentoFlag && voice->portamento) {
			if (voice->portamento > 0x1f)
				voice->pitch += voice->portamento & 0x1f;
			else
				voice->pitch -= voice->portamento;
		}
		voice->pitchBendFlag ^= 1;

		if (voice->pitchBendFlag && voice->pitchBendTime) {
			voice->pitchBendTime--;
			voice->pitch -= voice->pitchBendSpeed;
		}
		period += voice->pitch;

		if (period < 113) period = 113;
		else if (period > 3424) period = 3424;

		chan->period = period;
		chan->volume = voice->volume;

		if (voice->sample) {
			sample = voice->sample;
			chan->enabled = voice->enabled;
			chan->pointer = sample->loopPtr;
			chan->length  = sample->repeat;
		}
		voice = voice->next;
	}
}

//override
void FCPlayer_initialize(struct FCPlayer *self) {
	struct FCVoice *voice = &self->voices[0];
	self->super->initialize();

	ByteArray_set_position(self->seqs, 0);
	ByteArray_set_position(self->pats, 0);
	ByteArray_set_position(self->vols, 0);
	ByteArray_set_position(self->frqs, 0);

	while (voice) {
		voice->initialize();
		voice->channel = self->super.amiga->channels[voice->index];

		voice->pattern = self->seqs->readUnsignedByte() << 6;
		voice->transpose = self->seqs->readByte();
		voice->soundTranspose = self->seqs->readByte();

		voice = voice->next;
	}

	self->super.super.speed = self->seqs->readUnsignedByte();
	if (!self->super.super.speed) self->super.super.speed = 3;
	self->super.super.tick = self->super.super.speed;
}

//override
void FCPlayer_loader(struct FCPlayer *self, struct ByteArray *stream) {
	int i = 0;
	char *id; //:String, 
	int j; 
	int len; 
	int offset; 
	int position; 
	struct AmigaSample *sample;
	int size;
	int temp;
	int total;
	id = stream->readMultiByte(4, ENCODING);

	if (id == "SMOD") self->super.super.version = FUTURECOMP_10;
	else if (id == "FC14") self->super.super.version = FUTURECOMP_14;
	else return;

	ByteArray_set_position(stream, 4);
	self->length = stream->readUnsignedInt();
	ByteArray_set_position(stream, (self->super.super.version == FUTURECOMP_10 ? 100 : 180));
	self->seqs = new ByteArray();
	stream->readBytes(seqs, 0, length);
	self->length /= 13;

	ByteArray_set_position(stream, 12);
	len = stream->readUnsignedInt();
	ByteArray_set_position(stream, 8);
	ByteArray_set_position(stream, stream->readUnsignedInt());
	self->pats = new ByteArray();
	stream->readBytes(pats, 0, len);

	ByteArray_set_position(self->pats, pats->length);
	self->pats->writeByte(0);
	ByteArray_set_position(self->pats, 0);

	ByteArray_set_position(stream, 20);
	len = stream->readUnsignedInt();
	ByteArray_set_position(stream, 16);
	ByteArray_set_position(stream, stream->readUnsignedInt());
	self->frqs = new ByteArray();
	self->frqs->writeInt(0x01000000);
	self->frqs->writeInt(0x000000e1);
	stream->readBytes(frqs, 8, len);

	ByteArray_set_position(self->frqs, self->frqs->length);
	self->frqs->writeByte(0xe1);
	ByteArray_set_position(self->frqs, 0);

	ByteArray_set_position(stream, 28);
	len = stream->readUnsignedInt();
	ByteArray_set_position(stream, 24);
	ByteArray_set_position(stream, stream->readUnsignedInt());
	self->vols = new ByteArray();
	self->vols->writeInt(0x01000000);
	self->vols->writeInt(0x000000e1);
	stream->readBytes(vols, 8, len);

	ByteArray_set_position(stream, 32);
	size = stream->readUnsignedInt();
	ByteArray_set_position(stream, 40);

	if (self->super.super.version == FUTURECOMP_10) {
		samples = new Vector.<AmigaSample>(57, true);
		offset = 0;
	} else {
		samples = new Vector.<AmigaSample>(200, true);
		offset = 2;
	}

	for (i = 0; i < 10; ++i) {
		len = stream->readUnsignedShort() << 1;

		if (len > 0) {
			position = stream->position;
			ByteArray_set_position(stream, size);
			id = stream->readMultiByte(4, ENCODING);

			if (id == "SSMP") {
				temp = len;

				for (j = 0; j < 10; ++j) {
					stream->readInt();
					len = stream->readUnsignedShort() << 1;

					if (len > 0) {
						sample = new AmigaSample();
						sample->length = len + 2;
						sample->loop   = stream->readUnsignedShort();
						sample->repeat = stream->readUnsignedShort() << 1;

						if ((sample->loop + sample->repeat) > sample->length)
						sample->repeat = sample->length - sample->loop;

						if ((size + sample->length) > stream->length)
						sample->length = stream->length - size;

						sample->pointer = amiga->store(stream, sample->length, size + total);
						sample->loopPtr = sample->pointer + sample->loop;
						self->samples[(100 + (i * 10) + j)] = sample;
						total += sample->length;
						ByteArray_set_position_rel(stream, 6);
					} else {
						ByteArray_set_position_rel(stream, 10);
					}
				}

				size += (temp + 2);
				ByteArray_set_position(stream, position + 4);
			} else {
				ByteArray_set_position(stream, position);
				//FIXME
				sample = new AmigaSample();
				sample->length = len + offset;
				sample->loop   = stream->readUnsignedShort();
				sample->repeat = stream->readUnsignedShort() << 1;

				if ((sample->loop + sample->repeat) > sample->length)
					sample->repeat = sample->length - sample->loop;

				if ((size + sample->length) > stream->length)
					sample->length = stream->length - size;

				sample->pointer = self->super.amiga->store(stream, sample->length, size);
				sample->loopPtr = sample->pointer + sample->loop;
				self->samples[i] = sample;
				size += sample->length;
			}
		} else {
			ByteArray_set_position_rel(stream, 4);
		}
	}

	if (self->super.super.version == FUTURECOMP_10) {
		offset = 0; temp = 47;

		for (i = 10; i < 57; ++i) {
			//FIXME
			sample = new AmigaSample();
			sample->length  = WAVES[offset++] << 1;
			sample->loop    = 0;
			sample->repeat  = sample->length;

			position = self->super.amiga->memory->length;
			sample->pointer = position;
			sample->loopPtr = position;
			self->samples[i] = sample;

			len = position + sample->length;

			for (j = position; j < len; ++j)
				self->super.amiga->memory[j] = WAVES[temp++];
		}
	} else {
		ByteArray_set_position(stream, 36);
		size = stream->readUnsignedInt();
		ByteArray_set_position(stream, 100);

		for (i = 10; i < 90; ++i) {
			len = stream->readUnsignedByte() << 1;
			if (len < 2) continue;

			//FIXME
			sample = new AmigaSample();
			sample->length = len;
			sample->loop   = 0;
			sample->repeat = sample->length;

			if ((size + sample->length) > stream->length)
				sample->length = stream->length - size;

			sample->pointer = self->super.amiga->store(stream, sample->length, size);
			sample->loopPtr = sample->pointer;
			self->samples[i] = sample;
			size += sample->length;
		}
	}

	self->length *= 13;
}


const unsigned short PERIODS[] = {
        1712,1616,1524,1440,1356,1280,1208,1140,1076,1016, 960, 906,
         856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
         428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
         214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
         113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
        3424,3232,3048,2880,2712,2560,2416,2280,2152,2032,1920,1812,
        1712,1616,1524,1440,1356,1280,1208,1140,1076,1016, 960, 906,
         856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
         428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
         214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
         113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
};

const signed char WAVES[] = {
          16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
          16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
           8,   8,   8,   8,   8,   8,   8,   8,  16,   8,  16,  16,   8,   8,  24, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56,  63,
          55,  47,  39,  31,  23,  15,   7,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
          55,  47,  39,  31,  23,  15,   7,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72,  47,  39,  31,  23,  15,   7,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80,  39,  31,  23,  15,   7,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88,  31,  23,  15,   7,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,  23,  15,   7,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,  15,   7,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,   7,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,-120,  -1,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,-120,-128,   7,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,-120,-128,-120,  15,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,-120,-128,-120,-112,  23,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,-120,-128,-120,-112,-104,  31,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,-120,-128,-120,-112,-104, -96,  39,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,-120,-128,-120,-112,-104, -96, -88,  47,  55, -64,
         -64, -48, -40, -32, -24, -16,  -8,   0,  -8, -16, -24, -32, -40, -48, -56, -64,
         -72, -80, -88, -96,-104,-112,-120,-128,-120,-112,-104, -96, -88, -80,  55,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127, 127,
         127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
         127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127,-127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127,-127,-127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127,-127,-127,-127, 127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127, 127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127, 127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127, 127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127, 127, 127, 127, 127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,
        -127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127,-127, 127, 127, 127,-128,
        -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
        -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128, 127, 127,-128,
        -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,
        -128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128, 127,-128,
        -128,-128,-128,-128,-128,-128,-128, 127, 127, 127, 127, 127, 127, 127, 127,-128,
        -128,-128,-128,-128,-128,-128, 127, 127, 127, 127, 127, 127, 127, 127, 127,-128,
        -128,-128,-128,-128,-128, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-128,
        -128,-128,-128,-128, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-128,
        -128,-128,-128, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-128,
        -128,-128, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-128,
        -128, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-128,
        -128, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,-128,
        -128,-112,-104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16,  -8,   0,
           8,  16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  96, 104, 112, 127,-128,
        -128, -96, -80, -64, -48, -32, -16,   0,  16,  32,  48,  64,  80,  96, 112,  69,
          69, 121, 125, 122, 119, 112, 102,  97,  88,  83,  77,  44,  32,  24,  18,   4,
         -37, -45, -51, -58, -68, -75, -82, -88, -93, -99,-103,-109,-114,-117,-118,  69,
          69, 121, 125, 122, 119, 112, 102,  91,  75,  67,  55,  44,  32,  24,  18,   4,
          -8, -24, -37, -49, -58, -66, -80, -88, -92, -98,-102,-107,-108,-115,-125,   0,
           0,  64,  96, 127,  96,  64,  32,   0, -32, -64, -96,-128, -96, -64, -32,   0,
           0,  64,  96, 127,  96,  64,  32,   0, -32, -64, -96,-128, -96, -64, -32,-128,
        -128,-112,-104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16,  -8,   0,
           8,  16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  96, 104, 112, 127,-128,
        -128, -96, -80, -64, -48, -32, -16,   0,  16,  32,  48,  64,  80,  96, 112,
};

// FIXME figure out why the last row has only 15 entries
