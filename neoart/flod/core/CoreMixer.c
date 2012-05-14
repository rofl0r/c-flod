/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 3.0 - 2012/02/08

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/
#include "../flod.h"
#include "../flod_internal.h"
#include "CoreMixer.h"

void CoreMixer_defaults(struct CoreMixer* self) {
	CLASS_DEF_INIT();
	/* static initializers go here */
}

void CoreMixer_ctor(struct CoreMixer* self) {
	CLASS_CTOR_DEF(CoreMixer);
	/* original constructor code goes here */
	self->wave = ByteArray_new();
	self->wave->endian = BAE_LITTLE;
	self->bufferSize = 8192;
}

struct CoreMixer* CoreMixer_new(void) {
	CLASS_NEW_BODY(CoreMixer);
}

//js function reset
void CoreMixer_initialize(struct CoreMixer* self) {
	Sample* sample = self->buffer[0];

	self->samplesLeft = 0;
	self->remains     = 0;
	self->completed   = 0;

	while (sample) {
		sample->l = sample->r = 0.0;
		sample = sample->next;
	}
}

int CoreMixer_get_complete(struct CoreMixer* self) {
	return self->completed;
}

void CoreMixer_set_complete(struct CoreMixer* self, int value) {
	self->completed = value ^ self->player->loopSong;
}
void CoreMixer_reset(struct CoreMixer* self) {}
void CoreMixer_fast(struct CoreMixer* self, struct SampleDataEvent *e) {}
void CoreMixer_accurate(struct CoreMixer* self, struct SampleDataEvent *e) {}

int CoreMixer_get_bufferSize(struct CoreMixer* self) {
	return self->buffer->length; 
}

void CoreMixer_set_bufferSize(struct CoreMixer* self, int value) {
	int i = 0; int len = 0;
	if (value == len || value < 2048) return;

	if (!self->buffer) {
		self->buffer = new Vector.<Sample>(value, true);
	} else {
		len = self->buffer->length;
		self->buffer->fixed = false;
		self->buffer->length = value;
		self->buffer->fixed = true;
	}

	if (value > len) {
		self->buffer[len] = Sample_new();

		for (i = ++len; i < value; ++i)
		self->buffer[i] = self->buffer[i - 1].next = Sample_new();
	}
}

struct ByteArray* CoreMixer_waveform(struct CoreMixer* self) {
	struct ByteArray file = ByteArray_new();
	file->endian = BAE_LITTLE;

	file->writeUTFBytes("RIFF");
	file->writeInt(wave->length + 44);
	file->writeUTFBytes("WAVEfmt ");
	file->writeInt(16);
	file->writeShort(1);
	file->writeShort(2);
	file->writeInt(44100);
	file->writeInt(44100 << 2);
	file->writeShort(4);
	file->writeShort(16);
	file->writeUTFBytes("data");
	file->writeInt(wave->length);
	file->writeBytes(wave);

	ByteArray_set_position(file, 0);
	return file;
}

