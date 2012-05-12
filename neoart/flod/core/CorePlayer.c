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

CorePlayer_defaults(struct CorePlayer* self) {
	self->encoding = ENCODING_US_ASCII;
	self->title = "";
	self->soundPos = 0.0;
}

void CorePlayer_dtor(struct CorePlayer* self, struct CoreMixer *hardware) {
	hardware->player = this;
	self->hardware = hardware;
}

void CorePlayer_set_force(struct CorePlayer* self, int value) {
	self->version = 0;
}

void CorePlayer_set_ntsc(struct CorePlayer* self, int value) { }

void CorePlayer_set_stereo(struct CorePlayer* self, Number value) { }

CorePlayer_set_volume(struct CorePlayer* self, Number value) { }

CorePlayer_get_waveform(struct CorePlayer* self):ByteArray {
return self->hardware->waveform();
}

void CorePlayer_toggle(struct CorePlayer* self, int index) {

    CorePlayer_load(struct CorePlayer* self, stream:ByteArray):int {
      struct ZipFile* zip;
      self->hardware->reset();
      stream->position = 0;

      version  = 0;
      playSong = 0;
      lastSong = 0;

      if (stream->readUnsignedInt() == 67324752) {
        zip = new ZipFile(stream);
        stream = zip->uncompress(zip->entries[0]);
      }

      if (stream) {
        stream->endian = endian;
        stream->position = 0;
        loader(stream);
        if (version) setup();
      }
      return version;
    }


package neoart->flod->core {
  import flash.events.*;
  import flash.media.*;
  import flash.utils.*;
  import neoart.flip.*;

  public class CorePlayer extends EventDispatcher {


    CorePlayer_play(struct CorePlayer* self, processor:Sound = null):int {
      if (!version) return 0;
      if (soundPos == 0.0) initialize();
      sound = processor || new Sound();

      if (quality && (hardware is Soundblaster)) {
        sound->addEventListener(SampleDataEvent->SAMPLE_DATA, hardware->accurate);
      } else {
        sound->addEventListener(SampleDataEvent->SAMPLE_DATA, hardware->fast);
      }

      soundChan = sound->play(soundPos);
      soundChan->addEventListener(Event->SOUND_COMPLETE, completeHandler);
      soundPos = 0.0;
      return 1;
    }

void CorePlayer_pause(struct CorePlayer* self) {
      if (!version || !soundChan) return;
      soundPos = soundChan->position;
      removeEvents();
    }

void CorePlayer_stop(struct CorePlayer* self) {
      if (!version) return;
      if (soundChan) removeEvents();
      soundPos = 0.0;
      reset();
    }

void CorePlayer_process(struct CorePlayer* self) {

void CorePlayer_fast(struct CorePlayer* self) {

void CorePlayer_accurate(struct CorePlayer* self) {

void CorePlayer_setup(struct CorePlayer* self) {

    //js function reset
void CorePlayer_initialize(struct CorePlayer* self) {
      tick = 0;
      hardware->initialize();
      hardware->samplesTick = 110250 / tempo;
    }

void CorePlayer_reset(struct CorePlayer* self) {

void CorePlayer_loader(struct CorePlayer* self, stream:ByteArray) {

void CorePlayer_completeHandler(struct CorePlayer* self, e:Event) {
      stop();
      dispatchEvent(e);
    }

void CorePlayer_removeEvents(struct CorePlayer* self) {
      soundChan->stop();
      soundChan->removeEventListener(Event->SOUND_COMPLETE, completeHandler);
      soundChan->dispatchEvent(new Event(Event->SOUND_COMPLETE));

      if (quality) {
        sound->removeEventListener(SampleDataEvent->SAMPLE_DATA, hardware->accurate);
      } else {
        sound->removeEventListener(SampleDataEvent->SAMPLE_DATA, hardware->fast);
      }
    }
  }
}