/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.1 - 2012/04/09

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "Amiga.h"
#include "../flod_internal.h"
#include "../whittaker/DWPlayer.h"

void Amiga_defaults(struct Amiga* self) {
	CLASS_DEF_INIT();
	/* static initializers go here */	
	self->model = MODEL_A1200;
	self->loopLen = 4;
	self->clock = 0.0f;
	self->master = 0.00390625f;
}

void Amiga_ctor(struct Amiga* self) {
	CLASS_CTOR_DEF(Amiga);
	/* original constructor code goes here */	
	//super();
	
	PFUNC();
	
	CoreMixer_ctor(&self->super);
	self->super.type = CM_AMIGA;
	CoreMixer_set_bufferSize((struct CoreMixer*) self, 8192);
	self->filter = AmigaFilter_new();
	
	unsigned int i;
	for(i = 0; i < AMIGA_MAX_CHANNELS; i++) {
		AmigaChannel_ctor(&self->channels[i], i);
		if(i) self->channels[i -1].next = &self->channels[i];
	}
	/*
	self->channels = new Vector.<AmigaChannel>(4, true);

	self->channels[0] = AmigaChannel_new(0);
	self->channels[0].next = self->channels[1] = AmigaChannel_new(1);
	self->channels[1].next = self->channels[2] = AmigaChannel_new(2);
	self->channels[2].next = self->channels[3] = AmigaChannel_new(3); */
	
	// vtable
	self->super.fast = Amiga_fast;
	self->super.accurate = Amiga_fast; // simply use the fast one
}

struct Amiga* Amiga_new(void) {
	CLASS_NEW_BODY(Amiga);
}


void Amiga_set_volume(struct Amiga* self, int value) {
	if (value > 0) {
		if (value > 64) value = 64;
		self->master = ((float) value / 64.f) * 0.00390625f;
	} else {
		self->master = 0.0f;
	}
}

static void Amiga_memory_set_length(struct Amiga *self, unsigned len) {
	assert_dbg(len <= AMIGA_MAX_MEMORY);
	self->vector_count_memory = len;
}

// pointer default value: -1
int Amiga_store(struct Amiga* self, struct ByteArray *stream, int len, int pointer) {
	int add = 0; 
	int i = 0;
	int pos = ByteArray_get_position(stream);
	//int start = self->memory->length;
	int start = self->vector_count_memory;
	int total = 0;

	if (pointer > -1) ByteArray_set_position(stream, pointer);
	total = ByteArray_get_position(stream) + len;

	if (total >= ByteArray_get_length(stream)) {
		add = total - ByteArray_get_length(stream);
		len = ByteArray_get_length(stream) - ByteArray_get_position(stream);
	}
	
	len += start;
	Amiga_memory_set_length(self, len);
	//as3 memory.length is automatically increased + 1 when a byte gets written
	for (i = start; i < len; ++i)
		self->memory[i] = stream->readByte(stream);

	//self->memory->length += add;
	Amiga_memory_set_length(self, self->vector_count_memory + add); // FIXME dubious, why add and not len ?
	assert(self->vector_count_memory <= AMIGA_MAX_MEMORY);
	if (pointer > -1) ByteArray_set_position(stream, pos);
	return start;
}

//override
void Amiga_initialize(struct Amiga* self) {
	PFUNC();
	//self->super->initialize();
	//if(!self->super.player->record) ByteArray_clear(self->super.wave);
	AmigaFilter_initialize(self->filter);

	if (!self->memory_fixed) { 
		// we mimic the boolean behaviour of memory.fixed using that variable
		self->loopPtr = self->vector_count_memory;
		//self->loopPtr = 54686;
		Amiga_memory_set_length(self, self->vector_count_memory + self->loopLen);
		self->memory_fixed = true;
	}
	
	unsigned i;
	for(i = 0; i < AMIGA_MAX_CHANNELS; i++) AmigaChannel_initialize(&self->channels[i]);
}

//override
void Amiga_reset(struct Amiga* self) {
	PFUNC();
      //self->memory = new Vector.<int>();
      //memset(self->memory, 0, sizeof(self->memory));
      self->vector_count_memory = 0;
}

    //override
void Amiga_fast(struct Amiga* self, struct SampleDataEvent *e) {
	struct AmigaChannel *chan = NULL;
	//struct ByteArray *data = e->data;
	int i = 0;
	Number lvol = NAN;
	int mixed = 0;
	int mixLen = 0;
	int mixPos = 0;
	Number rvol = NAN;
	struct Sample *sample = NULL;
	int size = CoreMixer_get_bufferSize(&self->super);
	Number speed = NAN;
	int toMix = 0;
	Number value = NAN;
	
	PFUNC();

	if (self->super.completed) {
		if (!self->super.remains) return;
		size = self->super.remains;
	}

	while (mixed < size) {
		if (!self->super.samplesLeft) {

			self->super.player->process(self->super.player);
			self->super.samplesLeft = self->super.samplesTick;

			if (self->super.completed) {
				size = mixed + self->super.samplesTick;

				if (size > CoreMixer_get_bufferSize((struct CoreMixer*) self)) {
					self->super.remains = size - CoreMixer_get_bufferSize((struct CoreMixer*) self);
					size = CoreMixer_get_bufferSize((struct CoreMixer*) self);
				}
			}
		}

		toMix = self->super.samplesLeft;
		if ((mixed + toMix) >= size) toMix = size - mixed;
		mixLen = mixPos + toMix;
		chan = &self->channels[0];

		while (chan) {
			assert(mixPos < COREMIXER_MAX_BUFFER);
			sample = &self->super.buffer[mixPos];

			if (chan->audena && chan->audper > 60) {
				if (chan->mute) {
					chan->ldata = 0.0f;
					chan->rdata = 0.0f;
				}

				speed = chan->audper / self->clock;

				value = chan->audvol * self->master;
				lvol = value * (1 - chan->level);
				rvol = value * (1 + chan->level);

				for (i = mixPos; i < mixLen; ++i) {
					if (chan->delay) {
						chan->delay--;
					} else if (--chan->timer < 1.0f) { 
						if (!chan->mute) {
							//__asm__("int3");
							// FIXME this spot accesses data that has not previously been used
							// the commented assert statement enforces the correct bounds.
							//assert(chan->audloc < self->vector_count_memory);
							assert(chan->audloc < AMIGA_MAX_MEMORY);
							value = self->memory[chan->audloc] * 0.0078125f;
							chan->ldata = value * lvol;
							chan->rdata = value * rvol;
						}

						chan->audloc++;
						chan->timer += speed;

						if (chan->audloc >= chan->audcnt) {
							chan->audloc = chan->pointer;
							chan->audcnt = chan->pointer + chan->length;
						}
					}

					sample->l += chan->ldata;
					sample->r += chan->rdata;
					sample = sample->next;
				}
			} else {
				for (i = mixPos; i < mixLen; ++i) {
					sample->l += chan->ldata;
					sample->r += chan->rdata;
					sample = sample->next;
				}
			}
			chan = chan->next;
		}

		mixPos = mixLen;
		mixed += toMix;
		self->super.samplesLeft -= toMix;
	}

	sample = &self->super.buffer[0];

	for (i = 0; i < size; ++i) {
		AmigaFilter_process(self->filter, self->model, sample);
		
		//if (self->super.player->record) {
			// FIXME: this is all a hack
			// in the end only a wav (this here) should get written into data
			// and the backend decide whether it gets written to a wav file
			// or to an audio device.
			if (ByteArray_bytesAvailable(self->super.wave) >= 4) {
				self->super.wave->writeShort(self->super.wave, (sample->l * ((sample->l < 0) ? 32768 : 32767)));
				self->super.wave->writeShort(self->super.wave, (sample->r * ((sample->r < 0) ? 32768 : 32767)));
			} else {
				CoreMixer_set_complete(&self->super, 1);
			}
		//}
		// FIXME: this data is worthless, it is in the special flash sound format
		// which is one float in the range of -1 - 1 per channel
		// we can probably save a lot of calculations if we write the wave format directly,
		// instead of converting the flash format into wave.
		//data->writeFloat(data, sample->l);
		//data->writeFloat(data, sample->r);

		sample->l = sample->r = 0.0f;
		sample = sample->next;
	}

}

