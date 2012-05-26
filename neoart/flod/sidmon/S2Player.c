/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.0 - 2012/03/31

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "S2Player.h"
#include "../flod_internal.h"

void S2Player_process(struct S2Player* self);
void S2Player_initialize(struct S2Player* self);
void S2Player_loader(struct S2Player* self, struct ByteArray *stream);

void S2Player_defaults(struct S2Player* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void S2Player_ctor(struct S2Player* self, struct Amiga *amiga) {
	CLASS_CTOR_DEF(S2Player);
	// original constructor code goes here
	AmigaPlayer_ctor(&self->super, amiga);
	//super(amiga);
	
	//arpeggioFx = new Vector.<int>(4, true);
	//voices     = new Vector.<S2Voice>(4, true);
	unsigned i = 0;
	for(; i < 4; i++) {
		S2Voice_ctor(&self->voices[i]);
		if(i) self->voices[i - 1].next = &self->voices[i];
	}

	self->super.super.loader = S2Player_loader;
	self->super.super.process = S2Player_process;
	self->super.super.initialize = S2Player_initialize;
}

struct S2Player* S2Player_new(struct Amiga *amiga) {
	CLASS_NEW_BODY(S2Player, amiga);
}

static const unsigned short PERIODS = {0,
        5760,5424,5120,4832,4560,4304,4064,3840,3616,3424,3232,3048,
        2880,2712,2560,2416,2280,2152,2032,1920,1808,1712,1616,1524,
        1440,1356,1280,1208,1140,1076,1016, 960, 904, 856, 808, 762,
         720, 678, 640, 604, 570, 538, 508, 480, 453, 428, 404, 381,
         360, 339, 320, 302, 285, 269, 254, 240, 226, 214, 202, 190,
         180, 170, 160, 151, 143, 135, 127, 120, 113, 107, 101,  95,
};

//override
void S2Player_process(struct S2Player* self) {
	struct AmigaChannel *chan = 0;
	struct S2Instrument *instr = 0;
	struct SMRow *row = 0;
	struct S2Sample *sample = 0;
	int value = 0; 
	struct S2Voice *voice = &self->voices[0];
	
	self->arpeggioPos = ++(self->arpeggioPos) & 3;

	if (++(self->super.super.tick) >= self->super.super.speed) {
		self->super.tick = 0;

		while (voice) {
			chan = voice->channel;
			voice->enabled = voice->note = 0;

			if (!self->patternPos) {
				voice->step    = &self->tracks[int(self->trackPos + voice->index * self->length)];
				voice->pattern = voice->step->super.pattern;
				voice->speed   = 0;
			}
			if (--voice->speed < 0) {
				voice->row   = row = self->patterns[voice->pattern++];
				voice->speed = row->speed;

				if (row->super.note) {
					voice->enabled = 1;
					voice->note    = row->super.note + voice->step->super.transpose;
					chan->enabled  = 0;
				}
			}
			voice->pitchBend = 0;

			if (voice->note) {
				voice->waveCtr      = voice->sustainCtr     = 0;
				voice->arpeggioCtr  = voice->arpeggioPos    = 0;
				voice->vibratoCtr   = voice->vibratoPos     = 0;
				voice->pitchBendCtr = voice->noteSlideSpeed = 0;
				voice->adsrPos = 4;
				voice->volume  = 0;

				if (row->super.sample) {
					voice->instrument = row->super.sample;
					voice->instr  = self->instruments[int(voice->instrument + voice->step->soundTranspose)];
					voice->sample = self->samples[self->waves[voice->instr->wave]];
				}
				voice->original = voice->note + self->arpeggios[voice->instr->arpeggio];
				chan->period    = voice->period = PERIODS[voice->original];

				sample = voice->sample;
				chan->pointer = sample->super.pointer;
				chan->length  = sample->super.length;
				chan->enabled = voice->enabled;
				chan->pointer = sample->super.loopPtr;
				chan->length  = sample->super.repeat;
			}
			voice = voice->next;
		}

		if (++(self->patternPos) == self->patternLen) {
			self->patternPos = 0;

			if (++(self->trackPos) == self->length) {
				self->trackPos = 0;
				self->amiga->complete = 1;
			}
		}
	}
	voice = self->voices[0];

	while (voice) {
		if (!voice->sample) {
			voice = voice->next;
			continue;
		}
		chan   = voice->channel;
		sample = voice->sample;

		if (sample->negToggle) {
			voice = voice->next;
			continue;
		}
		sample->negToggle = 1;

		if (sample->negCtr) {
			sample->negCtr = --sample->negCtr & 31;
		} else {
			sample->negCtr = sample->negSpeed;
			if (!sample->negDir) {
				voice = voice->next;
				continue;
			}

			value = sample->negStart + sample->negPos;
			self->super.amiga->memory[value] = ~self->super.amiga->memory[value];
			sample->negPos += sample->negOffset;
			value = sample->negLen - 1;

			if (sample->negPos < 0) {
				if (sample->negDir == 2) {
					sample->negPos = value;
				} else {
					sample->negOffset = -sample->negOffset;
					sample->negPos += sample->negOffset;
				}
			} else if (value < sample->negPos) {
				if (sample->negDir == 1) {
					sample->negPos = 0;
				} else {
					sample->negOffset = -sample->negOffset;
					sample->negPos += sample->negOffset;
				}
			}
		}
		voice = voice->next;
	}
	voice = &self->voices[0];

	while (voice) {
		if (!voice->sample) {
			voice = voice->next;
			continue;
		}
		voice->sample->negToggle = 0;
		voice = voice->next;
	}
	voice = &self->voices[0];

	while (voice) {
		chan  = voice->channel;
		instr = voice->instr;

		switch (voice->adsrPos) {
			case 0:
				break;
			case 4:   //attack
				voice->volume += instr->attackSpeed;
				if (instr->attackMax <= voice->volume) {
					voice->volume = instr->attackMax;
					voice->adsrPos--;
				}
				break;
			case 3:   //decay
				if (!instr->decaySpeed) {
					voice->adsrPos--;
				} else {
					voice->volume -= instr->decaySpeed;
					if (instr->decayMin >= voice->volume) {
						voice->volume = instr->decayMin;
						voice->adsrPos--;
					}
				}
				break;
			case 2:   //sustain
				if (voice->sustainCtr == instr->sustain) voice->adsrPos--;
				else voice->sustainCtr++;
				break;
			case 1:   //release
				voice->volume -= instr->releaseSpeed;
				if (instr->releaseMin >= voice->volume) {
					voice->volume = instr->releaseMin;
					voice->adsrPos--;
				}
				break;
			default:
				break;
		}
		chan->volume = voice->volume >> 2;

		if (instr->waveLen) {
			if (voice->waveCtr == instr->waveDelay) {
				voice->waveCtr = instr->waveDelay - instr->waveSpeed;
				if (voice->wavePos == instr->waveLen) voice->wavePos = 0;
				else voice->wavePos++;

				voice->sample = sample = self->samples[self->waves[int(instr->wave + voice->wavePos)]];
				chan->pointer = sample->super.pointer;
				chan->length  = sample->super.length;
			} else
				voice->waveCtr++;
		}

		if (instr->arpeggioLen) {
			if (voice->arpeggioCtr == instr->arpeggioDelay) {
				voice->arpeggioCtr = instr->arpeggioDelay - instr->arpeggioSpeed;
				if (voice->arpeggioPos == instr->arpeggioLen) voice->arpeggioPos = 0;
				else voice->arpeggioPos++;

				value = voice->original + self->arpeggios[int(instr->arpeggio + voice->arpeggioPos)];
				voice->period = PERIODS[value];
			} else
				voice->arpeggioCtr++;
		}
		row = voice->row;

		if (self->super.super.tick) {
			switch (row->super.effect) {
				case 0:
					break;
				case 0x70:  //arpeggio
					self->arpeggioFx[0] = row->super.param >> 4;
					self->arpeggioFx[2] = row->super.param & 15;
					value = voice->original + self->arpeggioFx[self->arpeggioPos];
					voice->period = PERIODS[value];
					break;
				case 0x71:  //pitch up
					voice->pitchBend = -row->super.param;
					break;
				case 0x72:  //pitch down
					voice->pitchBend = row->super.param;
					break;
				case 0x73:  //volume up
					if (voice->adsrPos != 0) break;
					if (voice->instrument != 0) voice->volume = instr->attackMax;
					voice->volume += row->super.param << 2;
					if (voice->volume >= 256) voice->volume = -1;
					break;
				case 0x74:  //volume down
					if (voice->adsrPos != 0) break;
					if (voice->instrument != 0) voice->volume = instr->attackMax;
					voice->volume -= row->super.param << 2;
					if (voice->volume < 0) voice->volume = 0;
					break;
				default:
					break;
			}
		}

		switch (row->super.effect) {
			default:
			case 0:
				break;
			case 0x75:  //set adsr attack
				instr->attackMax   = row->super.param;
				instr->attackSpeed = row->super.param;
				break;
			case 0x76:  //set pattern length
				self->patternLen = row->super.param;
				break;
			case 0x7c:  //set volume
				chan->volume  = row->super.param;
				voice->volume = row->super.param << 2;
				if (voice->volume >= 255) voice->volume = 255;
				break;
			case 0x7f:  //set speed
				value = row->super.param & 15;
				if (value) self->super.super.speed = value;
				break;
		}

		if (instr->vibratoLen) {
			if (voice->vibratoCtr == instr->vibratoDelay) {
				voice->vibratoCtr = instr->vibratoDelay - instr->vibratoSpeed;
				if (voice->vibratoPos == instr->vibratoLen) voice->vibratoPos = 0;
				else voice->vibratoPos++;

				voice->period += self->vibratos[int(instr->vibrato + voice->vibratoPos)];
			} else
				voice->vibratoCtr++;
		}

		if (instr->pitchBend) {
			if (voice->pitchBendCtr == instr->pitchBendDelay) {
				voice->pitchBend += instr->pitchBend;
			} else
				voice->pitchBendCtr++;
		}

		if (row->super.param) {
			if (row->super.effect && row->super.effect < 0x70) {
				voice->noteSlideTo = PERIODS[int(row->super.effect + voice->step->super.transpose)];
				value = row->super.param;
				if ((voice->noteSlideTo - voice->period) < 0) value = -value;
					voice->noteSlideSpeed = value;
			}
		}

		if (voice->noteSlideTo && voice->noteSlideSpeed) {
			voice->period += voice->noteSlideSpeed;

			if ((voice->noteSlideSpeed < 0 && voice->period < voice->noteSlideTo) ||
				(voice->noteSlideSpeed > 0 && voice->period > voice->noteSlideTo)) {
				voice->noteSlideSpeed = 0;
				voice->period = voice->noteSlideTo;
			}
		}

		voice->period += voice->pitchBend;

		if (voice->period < 95) voice->period = 95;
		else if (voice->period > 5760) voice->period = 5760;

		chan->period = voice->period;
		voice = voice->next;
	}
}

//override
void S2Player_initialize(struct S2Player* self) {
	struct S2Voice *voice = &self->voices[0];
	self->super->initialize();

	self->super.super.speed      = self->speedDef;
	self->super.super.tick       = self->speedDef;
	self->trackPos   = 0;
	self->patternPos = 0;
	self->patternLen = 64;

	while (voice) {
		voice->initialize();
		voice->channel = self->super.amiga->channels[voice->index];
		voice->instr   = self->instruments[0];

		self->arpeggioFx[voice->index] = 0;
		voice = voice->next;
	}
}

//override
void S2Player_loader(struct S2Player* self, struct ByteArray *stream) {
	int higher = 0; 
	int i = 0;
	char id[32];
	struct S2Instrument *instr = 0;
	int j = 0;
	int len = 0; 
	//pointers:Vector.<int>;
	int pointers[]; // FIXME
	int position = 0; 
	int pos = 0; 
	struct SMRow *row = 0;
	struct S2Step *step = 0;
	struct S2Sample *sample = 0;
	int sampleData = 0;
	int value = 0;
	
	stream->position = 58;
	stream->readMultiByte(stream, id, 28);
	if (!is_str(id, "SIDMON II - THE MIDI VERSION")) return;

	stream->position = 2;
	self->length   = stream->readUnsignedByte();
	self->speedDef = stream->readUnsignedByte();
	self->samples  = new Vector.<S2Sample>(stream->readUnsignedShort() >> 6, true);

	stream->position = 14;
	len = stream->readUnsignedInt();
	self->tracks = new Vector.<S2Step>(len, true);
	stream->position = 90;

	for (; i < len; ++i) {
		step = new S2Step();
		step->super.pattern = stream->readUnsignedByte();
		if (step->super.pattern > higher) higher = step->super.pattern;
		self->tracks[i] = step;
	}

	for (i = 0; i < len; ++i) {
		step = self->tracks[i];
		step->super.transpose = stream->readByte();
	}

	for (i = 0; i < len; ++i) {
		step = self->tracks[i];
		step->soundTranspose = stream->readByte();
	}

	position = stream->position;
	stream->position = 26;
	len = stream->readUnsignedInt() >> 5;
	instruments = new Vector.<S2Instrument>(++len, true);
	stream->position = position;

	self->instruments[0] = new S2Instrument();

	for (i = 0; ++i < len;) {
		instr = new S2Instrument();
		instr->wave           = stream->readUnsignedByte() << 4;
		instr->waveLen        = stream->readUnsignedByte();
		instr->waveSpeed      = stream->readUnsignedByte();
		instr->waveDelay      = stream->readUnsignedByte();
		instr->arpeggio       = stream->readUnsignedByte() << 4;
		instr->arpeggioLen    = stream->readUnsignedByte();
		instr->arpeggioSpeed  = stream->readUnsignedByte();
		instr->arpeggioDelay  = stream->readUnsignedByte();
		instr->vibrato        = stream->readUnsignedByte() << 4;
		instr->vibratoLen     = stream->readUnsignedByte();
		instr->vibratoSpeed   = stream->readUnsignedByte();
		instr->vibratoDelay   = stream->readUnsignedByte();
		instr->pitchBend      = stream->readByte();
		instr->pitchBendDelay = stream->readUnsignedByte();
		stream->readByte();
		stream->readByte();
		instr->attackMax      = stream->readUnsignedByte();
		instr->attackSpeed    = stream->readUnsignedByte();
		instr->decayMin       = stream->readUnsignedByte();
		instr->decaySpeed     = stream->readUnsignedByte();
		instr->sustain        = stream->readUnsignedByte();
		instr->releaseMin     = stream->readUnsignedByte();
		instr->releaseSpeed   = stream->readUnsignedByte();
		self->instruments[i] = instr;
		stream->position += 9;
	}

	position = stream->position;
	stream->position = 30;
	len = stream->readUnsignedInt();
	waves = new Vector.<int>(len, true);
	stream->position = position;

	for (i = 0; i < len; ++i) self->waves[i] = stream->readUnsignedByte();

	position = stream->position;
	stream->position = 34;
	len = stream->readUnsignedInt();
	arpeggios = new Vector.<int>(len, true);
	stream->position = position;

	for (i = 0; i < len; ++i) self->arpeggios[i] = stream->readByte();

	position = stream->position;
	stream->position = 38;
	len = stream->readUnsignedInt();
	vibratos = new Vector.<int>(len, true);
	stream->position = position;

	for (i = 0; i < len; ++i) self->vibratos[i] = stream->readByte();

	len = self->samples->length;
	position = 0;

	for (i = 0; i < len; ++i) {
		sample = new S2Sample();
		stream->readUnsignedInt();
		sample->super.length    = stream->readUnsignedShort() << 1;
		sample->super.loop      = stream->readUnsignedShort() << 1;
		sample->super.repeat    = stream->readUnsignedShort() << 1;
		sample->negStart  = position + (stream->readUnsignedShort() << 1);
		sample->negLen    = stream->readUnsignedShort() << 1;
		sample->negSpeed  = stream->readUnsignedShort();
		sample->negDir    = stream->readUnsignedShort();
		sample->negOffset = stream->readShort();
		sample->negPos    = stream->readUnsignedInt();
		sample->negCtr    = stream->readUnsignedShort();
		stream->position += 6;
		sample->super.name      = stream->readMultiByte(32, ENCODING);

		sample->super.pointer = position;
		sample->super.loopPtr = position + sample->super.loop;
		position += sample->super.length;
		self->samples[i] = sample;
	}

	sampleData = position;
	len = ++higher;
	pointers = new Vector.<int>(++higher, true);
	for (i = 0; i < len; ++i) pointers[i] = stream->readUnsignedShort();

	position = stream->position;
	stream->position = 50;
	len = stream->readUnsignedInt();
	patterns = new Vector.<SMRow>();
	stream->position = position;
	j = 1;

	for (i = 0; i < len; ++i) {
		row   = new SMRow();
		value = stream->readByte();

		if (!value) {
			row->super.effect = stream->readByte();
			row->super.param  = stream->readUnsignedByte();
			i += 2;
		} else if (value < 0) {
			row->speed = ~value;
		} else if (value < 112) {
			row->super.note = value;
			value = stream->readByte();
			i++;

			if (value < 0) {
				row->speed = ~value;
			} else if (value < 112) {
				row->super.sample = value;
				value = stream->readByte();
				i++;

				if (value < 0) {
					row->speed = ~value;
				} else {
					row->super.effect = value;
					row->super.param  = stream->readUnsignedByte();
					i++;
				}
			} else {
				row->super.effect = value;
				row->super.param  = stream->readUnsignedByte();
				i++;
			}
		} else {
			row->super.effect = value;
			row->super.param  = stream->readUnsignedByte();
			i++;
		}

		self->patterns[pos++] = row;
		if ((position + pointers[j]) == stream->position) pointers[j++] = pos;
	}
	pointers[j] = self->patterns->length;
	self->patterns->fixed = true;

	if ((stream->position & 1) != 0) stream->position++;
	self->super.amiga->store(stream, sampleData);
	len = self->tracks.length;

	for (i = 0; i < len; ++i) {
		step = &self->tracks[i];
		step->super.pattern = pointers[step->super.pattern];
	}

	self->length++;
	self->super.super.version = 2;
}

