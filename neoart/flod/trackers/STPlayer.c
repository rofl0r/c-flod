/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.1 - 2012/04/13

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "STPlayer.h"
#include "../flod_internal.h"

const unsigned short PERIODS[] = {
        856,808,762,720,678,640,604,570,538,508,480,453,
        428,404,381,360,339,320,302,285,269,254,240,226,
        214,202,190,180,170,160,151,143,135,127,120,113,
        0,0,0
};


void STPlayer_defaults(struct STPlayer* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

/* amiga default is null */
void STPlayer_ctor(struct STPlayer* self, struct Amiga *amiga) {
	CLASS_CTOR_DEF(STPlayer, amiga);
	// original constructor code goes here
	//super(amiga);
	AmigaPlayer_ctor((struct AmigaPlayer *) self, amiga);

	self->track   = new Vector.<int>(128, true);
	self->samples = new Vector.<AmigaSample>(16, true);
	self->voices  = new Vector.<STVoice>(4, true);

	self->voices[0] = new STVoice(0);
	self->voices[0].next = voices[1] = new STVoice(1);
	self->voices[1].next = voices[2] = new STVoice(2);
	self->voices[2].next = voices[3] = new STVoice(3);	
}

struct STPlayer* STPlayer_new(struct Amiga *amiga) {
	CLASS_NEW_BODY(STPlayer, amiga);
}

static int isLegal(char *text) {
	int i = 0;
	if (!text[i]) return 0;

	while(text[i]) {
		if (text[i] < 32 || text[i] > 127) return 0;
		i++;
	}
	return 1;
}


//override
void STPlayer_set_force(struct STPlayer* self, int value) {
	if (value < ULTIMATE_SOUNDTRACKER)
		value = ULTIMATE_SOUNDTRACKER;
	else if (value > DOC_SOUNDTRACKER_20)
		value = DOC_SOUNDTRACKER_20;

	self->super.super.version = value;
}

//override
void STPlayer_set_ntsc(struct STPlayer* self, int value) {
	AmigaPlayer_set_ntsc(&self->super, value);

	if (self->super.super.version < DOC_SOUNDTRACKER_9)
		self->super.amiga->super.samplesTick = (240 - self->super.super.tempo) * (value ? 7.5152005551f : 7.58437970472f);
}

//override
void STPlayer_process(struct STPlayer* self) {
	struct AmigaChannel *chan = NULL;
	struct AmigaRow *row = NULL;
	struct AmigaSample *sample = NULL;
	int value; 
	struct STVoice *voice = &self->voices[0];

	if (!self->super.super.tick) {
		value = self->track[self->trackPos] + self->patternPos;

		while (voice) {
			chan = voice->channel;
			voice->enabled = 0;

			row = self->patterns[int(value + voice->index)];
			voice->period = row->note;
			voice->effect = row->effect;
			voice->param  = row->param;

			if (row->sample) {
				sample = voice->sample = self->samples[row->sample];

				if (((self->super.super.version & 2) == 2) && voice->effect == 12) 
					AmigaChannel_set_volume(chan, voice->param);
				else 
					AmigaChannel_set_volume(chan, sample->volume);
			} else {
				sample = voice->sample;
			}

			if (voice->period) {
				voice->enabled = 1;

				AmigaChannel_set_enabled(chan, 0);
				chan->pointer = sample->pointer;
				chan->length  = sample->length;
				voice->last = voice->period;
				AmigaChannel_set_period(chan, voice->period);
			}

			if (voice->enabled) AmigaChannel_set_enabled(chan, 1);
			chan->pointer = sample->loopPtr;
			chan->length  = sample->repeat;

			if (self->super.super.version < DOC_SOUNDTRACKER_20) {
				voice = voice->next;
				continue;
			}

			switch (voice->effect) {
				case 11:  //position jump
					self->trackPos = voice->param - 1;
					self->jumpFlag ^= 1;
					break;
				case 12:  //set volume
					AmigaChannel_set_volume(chan, voice->param);
					break;
				case 13:  //pattern break
					self->jumpFlag ^= 1;
					break;
				case 14:  //set filter
					self->super.amiga->filter->active = voice->param ^ 1;
					break;
				case 15:  //set speed
					if (!voice->param) break;
					self->super.super.speed = voice->param & 0x0f;
					self->super.super.tick = 0;
					break;
				default:
					break;
			}
			voice = voice->next;
		}
	} else {
		while (voice) {
			if (!voice->param) {
				voice = voice->next;
				continue;
			}
			chan = voice->channel;

			if (self->super.super.version == ULTIMATE_SOUNDTRACKER) {
				if (voice->effect == 1) {
					STPlayer_arpeggio(self, voice);
				} else if (voice->effect == 2) {
					value = voice->param >> 4;

					if (value) voice->period += value;
					else voice->period -= (voice->param & 0x0f);

					AmigaChannel_set_period(chan, voice->period);
				}
			} else {
				switch (voice->effect) {
					case 0: //arpeggio
						STPlayer_arpeggio(self, voice);
						break;
					case 1: //portamento up
						voice->last -= voice->param & 0x0f;
						if (voice->last < 113) voice->last = 113;
						AmigaChannel_set_period(chan, voice->last);
						break;
					case 2: //portamento down
						voice->last += voice->param & 0x0f;
						if (voice->last > 856) voice->last = 856;
						AmigaChannel_set_period(chan, voice->last);
						break;
					default:
						break;
				}

				if ((self->super.super.version & 2) != 2) {
					voice = voice->next;
					continue;
				}

				switch (voice->effect) {
					case 12:  //set volume
						AmigaChannel_set_volume(chan, voice->param);
						break;
					case 14:  //set filter
						self->super.amiga->filter->active = 0;
						break;
					case 15:  //set speed
						self->super.super.speed = voice->param & 0x0f;
						break;
					default:
						break;
				}
			}
			voice = voice->next;
		}
	}

	if (++(self->super.super.tick) == self->super.super.speed) {
		self->super.super.tick = 0;
		self->patternPos += 4;

		if (self->patternPos == 256 || self->jumpFlag) {
			self->patternPos = self->jumpFlag = 0;

			if (++(self->trackPos) == self->length) {
				self->trackPos = 0;
				CoreMixer_set_complete(&self->super.amiga->super, 1);
			}
		}
	}
}

//override
void STPlayer_initialize(struct STPlayer* self) {
	struct STVoice *voice = &self->voices[0];
	CorePlayer_initialize(&self->super.super);
	//super->initialize();
	AmigaPlayer_set_ntsc((struct AmigaPlayer*) self, self->super.standard);

	self->super.super.speed = 6;
	self->trackPos   = 0;
	self->patternPos = 0;
	self->jumpFlag   = 0;

	while (voice) {
		STVoice_initialize(voice);
		voice->channel = &self->super.amiga->channels[voice->index];
		voice->sample  = &self->samples[0];
		voice = voice->next;
	}
}

//override
void STPlayer_loader(struct STPlayer* self, struct ByteArray *stream) {
	int higher = 0;
	int i = 0; 
	int j = 0; 
	struct AmigaRow *row = NULL;
	struct AmigaSample *sample = NULL;
	int score = 0; 
	int size = 0; 
	int value = 0;
	
	if (stream->length < 1626) return;

	self->super.super.title = stream->readMultiByte(20, ENCODING);
	score += isLegal(self->super.super.title);

	self->super.super.version = ULTIMATE_SOUNDTRACKER;
	stream->position = 42;

	for (i = 1; i < 16; ++i) {
		value = stream->readUnsignedShort();

		if (!value) {
			self->samples[i] = null;
			stream->position += 28;
			continue;
		}

		sample = new AmigaSample();
		stream->position -= 24;

		sample->name = stream->readMultiByte(22, ENCODING);
		sample->length = value << 1;
		stream->position += 3;

		sample->volume = stream->readUnsignedByte();
		sample->loop   = stream->readUnsignedShort();
		sample->repeat = stream->readUnsignedShort() << 1;

		stream->position += 22;
		sample->pointer = size;
		size += sample->length;
		self->samples[i] = sample;

		score += isLegal(sample->name);
		if (sample->length > 9999) self->super.super.version = MASTER_SOUNDTRACKER;
	}

	stream->position = 470;
	self->length = stream->readUnsignedByte();
	self->super.super.tempo  = stream->readUnsignedByte();

	for (i = 0; i < 128; ++i) {
		value = stream->readUnsignedByte() << 8;
		if (value > 16384) score--;
		self->track[i] = value;
		if (value > higher) higher = value;
	}

	stream->position = 600;
	higher += 256;
	patterns = new Vector.<AmigaRow>(higher, true);

	i = (stream->length - size - 600) >> 2;
	if (higher > i) higher = i;

	for (i = 0; i < higher; ++i) {
		row = new AmigaRow();

		row->note   = stream->readUnsignedShort();
		value       = stream->readUnsignedByte();
		row->param  = stream->readUnsignedByte();
		row->effect = value & 0x0f;
		row->sample = value >> 4;

		self->patterns[i] = row;

		if (row->effect > 2 && row->effect < 11) score--;
		if (row->note) {
			if (row->note < 113 || row->note > 856) score--;
		}

		if (row->sample)
		if (row->sample > 15 || !self->samples[row->sample]) {
			if (row->sample > 15) score--;
			row->sample = 0;
		}

		if (row->effect > 2 || (!row->effect && row->param != 0))
			self->super.super.version = DOC_SOUNDTRACKER_9;

		if (row->effect == 11 || row->effect == 13)
			self->super.super.version = DOC_SOUNDTRACKER_20;
	}

	Amiga_store(self->super.amiga, stream, size, -1);

	for (i = 1; i < 16; ++i) {
		sample = &self->samples[i];
		if (!sample) continue;

		if (sample->loop) {
			sample->loopPtr = sample->pointer + sample->loop;
			sample->pointer = sample->loopPtr;
			sample->length  = sample->repeat;
		} else {
			sample->loopPtr = self->super.amiga->vector_count_memory;
			sample->repeat  = 2;
		}

		size = sample->pointer + 4;
		for (j = sample->pointer; j < size; ++j) self->super.amiga->memory[j] = 0;
	}

	sample = new AmigaSample();
	sample->pointer = sample->loopPtr = self->super.amiga->vector_count_memory;
	sample->length  = sample->repeat  = 2;
	self->samples[0] = sample;

	if (score < 1) self->super.super.version = 0;
}

void STPlayer_arpeggio(struct STPlayer* self, struct STVoice *voice) {
	struct AmigaChannel *chan = voice->channel;
	int i = 0;
	itn param = self->super.super.tick % 3;

	if (!param) {
		AmigaChannel_set_period(chan, voice->last);
		return;
	}

	if (param == 1) param = voice->param >> 4;
	else param = voice->param & 0x0f;

	while (voice->last != PERIODS[i]) i++;
	AmigaChannel_set_period(chan, PERIODS[i + param]);
}
