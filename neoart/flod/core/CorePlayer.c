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

#include "CorePlayer.h"
#include "../flod_internal.h"
#include "Amiga.h"
#include "Soundblaster.h"

void CorePlayer_defaults(struct CorePlayer* self) {
	CLASS_DEF_INIT();
	// static initializers go here
	self->encoding = ENCODING_US_ASCII;
	self->title = "";
	self->soundPos = 0.0;
}

void CorePlayer_ctor(struct CorePlayer* self, struct CoreMixer *hardware) {
	CLASS_CTOR_DEF(CorePlayer);
	// original constructor code goes here
	hardware->player = this;
	self->hardware = hardware;
	
	//add vtable
	
}

struct CorePlayer* CorePlayer_new(struct CoreMixer *hardware) {
	CLASS_NEW_BODY(CorePlayer, hardware);
}

void CorePlayer_set_force(struct CorePlayer* self, int value) {
	self->version = 0;
}

void CorePlayer_set_ntsc(struct CorePlayer* self, int value) { }

void CorePlayer_set_stereo(struct CorePlayer* self, Number value) { }

void CorePlayer_set_volume(struct CorePlayer* self, Number value) { }

struct ByteArray *CorePlayer_get_waveform(struct CorePlayer* self) {
	return self->hardware->waveform();
}

void CorePlayer_toggle(struct CorePlayer* self, int index) {}

int CorePlayer_load(struct CorePlayer* self, struct ByteArray *stream) {
	self->hardware->reset();
	ByteArray_set_position(stream, 0);

	self->version  = 0;
	self->playSong = 0;
	self->lastSong = 0;
#ifdef SUPPORT_COMPRESSION
	struct ZipFile* zip;	
	if (stream->readUnsignedInt() == 67324752) {
		zip = ZipFile_new(stream);
		stream = zip->uncompress(zip->entries[0]);
	}
#endif

	if (stream) {
		stream->endian = endian;
		ByteArray_set_position(stream, 0);
		self->loader(stream);
		if (self->version) self->setup();
	}
	return self->version;
}

/* processor default : NULL */
int CorePlayer_play(struct CorePlayer* self, struct Sound *processor) {
	if (!self->version) return 0;
	if (self->soundPos == 0.0) {
		//self->initialize();
		CorePlayer_initialize(self);
	}
	self->sound = processor ? processor : Sound_new();

	if (self->quality && (self->hardware->type == CM_SOUNDBLASTER)) {
		self->sound->addEventListener(SampleDataEvent->SAMPLE_DATA, hardware->accurate);
	} else {
		self->sound->addEventListener(SampleDataEvent->SAMPLE_DATA, hardware->fast);
	}

	self->soundChan = self->sound->play(soundPos);
	self->soundChan->addEventListener(Event->SOUND_COMPLETE, completeHandler);
	self->soundPos = 0.0;
	return 1;
}

void CorePlayer_pause(struct CorePlayer* self) {
	if (!self->version || !self->soundChan) return;
	self->soundPos = self->soundChan->position;
	self->removeEvents();
}

void CorePlayer_stop(struct CorePlayer* self) {
	if (!self->version) return;
	if (self->soundChan) self->removeEvents();
	self->soundPos = 0.0;
	self->reset();
}

void CorePlayer_process(struct CorePlayer* self) {}

void CorePlayer_fast(struct CorePlayer* self) {}

void CorePlayer_accurate(struct CorePlayer* self) {}

void CorePlayer_setup(struct CorePlayer* self) {}

    //js function reset
void CorePlayer_initialize(struct CorePlayer* self) {
	self->tick = 0;
	CoreMixer_initialize(self->hardware);
	if(self->hardware->type == CM_AMIGA)
		Amiga_initialize((struct Amiga*) self->hardware);
	else if(self->hardware->type == CM_SOUNDBLASTER)
		Soundblaster_initialize((struct Soundblaster*) self->hardware);
	else
		abort();
	//self->hardware->initialize();
	self->hardware->samplesTick = 110250 / tempo;
}

void CorePlayer_reset(struct CorePlayer* self) { }

void CorePlayer_loader(struct CorePlayer* self, struct ByteArray *stream) { }

void CorePlayer_completeHandler(struct CorePlayer* self, struct Event *e) {
	CorePlayer_stop(self);
	self->dispatchEvent(e);
}

void CorePlayer_removeEvents(struct CorePlayer* self) {
	self->soundChan->stop();
	self->soundChan->removeEventListener(Event->SOUND_COMPLETE, completeHandler);
	self->soundChan->dispatchEvent(new Event(Event->SOUND_COMPLETE));

	if (self->quality) {
		self->sound->removeEventListener(self->SampleDataEvent->SAMPLE_DATA, self->hardware->accurate);
	} else {
		self->sound->removeEventListener(self->SampleDataEvent->SAMPLE_DATA, self->hardware->fast);
	}
}

