/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.0 - 2012/03/09

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "RHPlayer.h"
#include "../flod_internal.h"

void RHPlayer_loader(struct RHPlayer* self, struct ByteArray *stream);
void RHPlayer_initialize(struct RHPlayer* self);
void RHPlayer_process(struct RHPlayer* self);

void RHPlayer_defaults(struct RHPlayer* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void RHPlayer_ctor(struct RHPlayer* self, struct Amiga *amiga) {
	CLASS_CTOR_DEF(RHPlayer);
	// original constructor code goes here
	//super(amiga);
	AmigaPlayer_ctor(&self->super, amiga);
	
	/*voices = new Vector.<RHVoice>(4, true);
	voices[3] = new RHVoice(3,8);
	voices[3].next = voices[2] = new RHVoice(2,4);
	voices[2].next = voices[1] = new RHVoice(1,2);
	voices[1].next = voices[0] = new RHVoice(0,1); */
	
	unsigned i = RHPLAYER_MAX_VOICES;
	while(1) {
		RHVoice_ctor(&self->voices[i], i, 1 << i);
		if(i) self->voices[i].next = &self->voices[i - 1];
		else break;
		i--;
	}
	
	
	//vtable
	self->super.super.loader = RHPlayer_loader;
	self->super.super.initialize = RHPlayer_initialize;
	self->super.super.process = RHPlayer_process;
	
	self->super.super.min_filesize = 8;
}

struct RHPlayer* RHPlayer_new(struct Amiga *amiga) {
	CLASS_NEW_BODY(RHPlayer, amiga);
}


//override
void RHPlayer_process(struct RHPlayer* self) {
	struct AmigaChannel *chan = 0;
	int loop = 0;
	struct RHSample *sample = 0;
	int value = 0; 
	struct RHVoice *voice = &self->voices[3];

	while (voice) {
		chan = voice->channel;
		ByteArray_set_position(self->stream, voice->patternPos);
		sample = voice->sample;

		if (!voice->busy) {
			voice->busy = 1;

			if (sample->super.loopPtr == 0) {
				chan->pointer = self->super.amiga->loopPtr;
				chan->length  = self->super.amiga->loopLen;
			} else if (sample->super.loopPtr > 0) {
				chan->pointer = sample->super.pointer + sample->super.loopPtr;
				chan->length  = sample->super.length  - sample->super.loopPtr;
			}
		}

		if (--voice->tick == 0) {
			voice->flags = 0;
			loop = 1;

			while (loop) {
				value = self->stream->readByte(self->stream);

				if (value < 0) {
					switch (value) {
						case -121:
							if (self->super.super.variant == 3) voice->volume = self->stream->readUnsignedByte(self->stream);
							break;
						case -122:
							if (self->super.super.variant == 4) voice->volume = self->stream->readUnsignedByte(self->stream);
							break;
						case -123:
							if (self->super.super.variant > 1) self->super.amiga->complete = 1;
							break;
						case -124:
							ByteArray_set_position(self->stream, voice->trackPtr + voice->trackPos);
							value = self->stream->readUnsignedInt(self->stream);
							voice->trackPos += 4;

							if (!value) {
								ByteArray_set_position(self->stream, voice->trackPtr);
								value = self->stream->readUnsignedInt(self->stream);
								voice->trackPos = 4;

								if (!self->super.super.loopSong) {
									self->complete &= ~(voice->bitFlag);
									if (!self->complete) self->super.amiga->complete = 1;
								}
							}

							ByteArray_set_position(self->stream, value);
							break;
						case -125:
							if (self->super.super.variant == 4) voice->flags |= 4;
							break;
						case -126:
							voice->tick = self->song->speed * self->stream->readByte(self->stream);
							voice->patternPos = ByteArray_get_position(self->stream);

							chan->pointer = self->super.amiga->loopPtr;
							chan->length  = self->super.amiga->loopLen;
							loop = 0;
							break;
						case -127:
							voice->portaSpeed = self->stream->readByte(self->stream);
							voice->flags |= 1;
							break;
						case -128:
							value = self->stream->readByte(self->stream);
							if (value < 0) value = 0;
							voice->sample = sample = self->samples[value];
							voice->vibratoPtr = self->vibrato + sample->vibrato;
							voice->vibratoPos = voice->vibratoPtr;
							break;
						default:
							break;
					}
				} else {
					voice->tick = self->song->speed * value;
					voice->note = self->stream->readByte(self->stream);
					voice->patternPos = ByteArray_get_position(self->stream);

					voice->synthPos = sample->loPos;
					voice->vibratoPos = voice->vibratoPtr;

					chan->pointer = sample->super.pointer;
					chan->length  = sample->super.length;
					AmigaChannel_set_volume(chan, (voice->volume) ? voice->volume : sample->super.volume);

					ByteArray_set_position(self->stream, self->periods + (voice->note << 1));
					value = self->stream->readUnsignedShort(self->stream) * sample->relative;
					voice->period = (value >> 10);
					AmigaChannel_set_period(chan, voice->period);

					AmigaChannel_set_enabled(chan, 1);
					voice->busy = loop = 0;
				}
			}
		} else {
			if (voice->tick == 1) {
				if (self->super.super.variant != 4 || !(voice->flags & 4))
				AmigaChannel_set_enabled(chan, 0);
			}

			if (voice->flags & 1) {
				voice->period += voice->portaSpeed;
				AmigaChannel_set_period(chan, voice->period);
			}

			if (sample->divider) {
				ByteArray_set_position(self->stream, voice->vibratoPos);
				value = self->stream->readByte(self->stream);

				if (value == -124) {
					ByteArray_set_position(self->stream, voice->vibratoPtr);
					value = self->stream->readByte(self->stream);
				}

				voice->vibratoPos = ByteArray_get_position(self->stream);
				value = (voice->period / sample->divider) * value;
				AmigaChannel_set_period(chan, voice->period + value);
			}
		}

		if (sample->hiPos) {
			value = 0;

			if (voice->flags & 2) {
				voice->synthPos--;

				if (voice->synthPos <= sample->loPos) {
				voice->flags &= -3;
				value = 60;
				}
			} else {
				voice->synthPos++;

				if (voice->synthPos > sample->hiPos) {
					voice->flags |= 2;
					value = 60;
				}
			}

			self->super.amiga->memory[int(sample->super.pointer + voice->synthPos)] = value;
		}

		voice = voice->next;
	}
}

//override
void RHPlayer_initialize(struct RHPlayer* self) {
	int i = 0; 
	int j = 0; 
	struct RHSample *sample = 0;
	struct RHVoice *voice = &self->voices[3];
	
	self->super->initialize();

	self->song = self->songs[self->super.super.playSong];
	self->complete = 15;

	for (i = 0; i < self->samples->length; ++i) {
		sample = self->samples[i];

		if (sample->wave) {
			for (j = 0; j < sample->super.length; ++j)
				self->super.amiga->memory[int(sample->super.pointer + j)] = sample->wave[j];
		}
	}

	while (voice) {
		voice->initialize();
		voice->channel = self->super.amiga->channels[voice->index];

		voice->trackPtr = self->song->tracks[voice->index];
		voice->trackPos = 4;

		ByteArray_set_position(self->stream, voice->trackPtr);
		voice->patternPos = self->stream->readUnsignedInt(self->stream);

		voice = voice->next;
	}
}

//override
void RHPlayer_loader(struct RHPlayer* self, struct ByteArray *stream) {
	int i = 0; 
	int j = 0; 
	int len = 0; 
	int pos = 0; 
	struct RHSample *sample = 0;
	int samplesData = 0; 
	int samplesHeaders = 0; 
	int samplesLen = 0; 
	struct RHSong *song = 0;
	int songsHeaders = 0; 
	int wavesHeaders = 0; 
	int wavesPointers = 0; 
	int value = 0;
	
	ByteArray_set_position(stream, 44);

	while (ByteArray_get_position(stream) < 1024) {
		value = stream->readUnsignedShort(stream);

		if (value == 0x7e10 || value == 0x7e20) {                               //moveq #16,d7 || moveq #32,d7
			value = stream->readUnsignedShort(stream);

			if (value == 0x41fa) {                                                //lea $x,a0
				i = ByteArray_get_position(stream) + stream->readUnsignedShort(stream);
				value = stream->readUnsignedShort(stream);

				if (value == 0xd1fc) {                                              //adda->l
					samplesData = i + stream->readUnsignedInt(stream);
					self->super.amiga->loopLen = 64;
					ByteArray_set_position_rel(stream, +2);
				} else {
					samplesData = i;
					self->super.amiga->loopLen = 512;
				}

				samplesHeaders = ByteArray_get_position(stream) + stream->readUnsignedShort(stream);
				value = stream->readUnsignedByte(stream);
				if (value == 0x72) samplesLen = stream->readUnsignedByte(stream);           //moveq #x,d1
			}
		} else if (value == 0x51c9) {                                           //dbf d1,x
			ByteArray_set_position_rel(stream, +2);
			value = stream->readUnsignedShort(stream);

			if (value == 0x45fa) {                                                //lea $x,a2
				wavesPointers = ByteArray_get_position(stream) + stream->readUnsignedShort(stream);
				ByteArray_set_position_rel(stream, +2);

				while (1) {
					value = stream->readUnsignedShort(stream);

					if (value == 0x4bfa) {                                            //lea $x,a5
						wavesHeaders = ByteArray_get_position(stream) + stream->readUnsignedShort(stream);
						break;
					}
				}
			}
		} else if (value == 0xc0fc) {                                           //mulu->w #x,d0
			ByteArray_set_position_rel(stream, +2);
			value = stream->readUnsignedShort(stream);

			if (value == 0x41eb)                                                  //lea $x(a3),a0
				songsHeaders = stream->readUnsignedShort(stream);
		} else if (value == 0x346d) {                                           //movea->w x(a5),a2
			ByteArray_set_position_rel(stream, +2);
			value = stream->readUnsignedShort(stream);

			if (value == 0x49fa)                                                  //lea $x,a4
			self->vibrato = ByteArray_get_position(stream) + stream->readUnsignedShort(stream);
		} else if (value == 0x4240) {                                           //clr->w d0
			value = stream->readUnsignedShort(stream);

			if (value == 0x45fa) {                                                //lea $x,a2
				self->periods = ByteArray_get_position(stream) + stream->readUnsignedShort(stream);
				break;
			}
		}
	}

	if (!samplesHeaders || !samplesData || !samplesLen || !songsHeaders) return;

	ByteArray_set_position(stream, samplesData);
	samples = new Vector.<RHSample>();
	samplesLen++;

	for (i = 0; i < samplesLen; ++i) {
		sample = new RHSample();
		sample->super.length   = stream->readUnsignedInt(stream);
		sample->relative = 3579545 / stream->readUnsignedShort(stream);
		sample->super.pointer  = self->super.amiga->store(stream, sample->super.length);
		self->samples[i] = sample;
	}

	ByteArray_set_position(stream, samplesHeaders);

	for (i = 0; i < samplesLen; ++i) {
		sample = self->samples[i];
		ByteArray_set_position_rel(stream, +4);
		sample->super.loopPtr = stream->readInt(stream);
		ByteArray_set_position_rel(stream, +6);
		sample->super.volume = stream->readUnsignedShort(stream);

		if (wavesHeaders) {
			sample->divider = stream->readUnsignedShort(stream);
			sample->vibrato = stream->readUnsignedShort(stream);
			sample->hiPos   = stream->readUnsignedShort(stream);
			sample->loPos   = stream->readUnsignedShort(stream);
			ByteArray_set_position_rel(stream, +8);
		}
	}

	if (wavesHeaders) {
		ByteArray_set_position(stream, wavesHeaders);
		i = (wavesHeaders - samplesHeaders) >> 5;
		len = i + 3;
		self->super.super.variant = 1;

		if (i >= samplesLen) {
			for (j = samplesLen; j < i; ++j)
				self->samples[j] = new RHSample();
		}

		for (; i < len; ++i) {
			sample = new RHSample();
			ByteArray_set_position_rel(stream, +4);
			sample->super.loopPtr   = stream->readInt(stream);
			sample->super.length    = stream->readUnsignedShort(stream);
			sample->relative  = stream->readUnsignedShort(stream);

			ByteArray_set_position_rel(stream, +2);
			sample->super.volume  = stream->readUnsignedShort(stream);
			sample->divider = stream->readUnsignedShort(stream);
			sample->vibrato = stream->readUnsignedShort(stream);
			sample->hiPos   = stream->readUnsignedShort(stream);
			sample->loPos   = stream->readUnsignedShort(stream);

			pos = ByteArray_get_position(stream);
			ByteArray_set_position(stream, wavesPointers);
			ByteArray_set_position(stream, stream->readInt(stream));

			sample->super.pointer = self->super.amiga->memory->length;
			self->super.amiga->memory->length += sample->super.length;
			sample->wave = new Vector.<int>(sample->length, true);

			for (j = 0; j < sample->super.length; ++j)
			sample->wave[j] = stream->readByte(stream);

			self->samples[i] = sample;
			wavesPointers += 4;
			ByteArray_set_position(stream, pos);
		}
	}

	self->samples->fixed = true;

	ByteArray_set_position(stream, songsHeaders);
	self->songs = new Vector.<RHSong>();
	value = 65536;

	while (1) {
		song = new RHSong();
		ByteArray_set_position_rel(stream, +1);
		song->tracks = new Vector.<int>(4, true);
		song->speed = stream->readUnsignedByte(stream);

		for (i = 0; i < 4; ++i) {
			j = stream->readUnsignedInt(stream);
			if (j < value) value = j;
			song->tracks[i] = j;
		}

		self->songs->push(song);
		if ((value - ByteArray_get_position(stream)) < 18) break;
	}

	self->songs->fixed = true;
	self->super.super.lastSong = self->songs->length - 1;

	stream->length = samplesData;
	ByteArray_set_position(stream, 0x160);

	while (ByteArray_get_position(stream) < 0x200) {
		value = stream->readUnsignedShort(stream);

		if (value == 0xb03c) {                                                  //cmp->b #x,d0
			value = stream->readUnsignedShort(stream);

			if (value == 0x0085) {                                                //-123
				self->super.super.variant = 2;
			} else if (value == 0x0086) {                                         //-122
				self->super.super.variant = 4;
			} else if (value == 0x0087) {                                         //-121
				self->super.super.variant = 3;
			}
		}
	}

	self->stream = stream;
	self->super.super.version = 1;
}