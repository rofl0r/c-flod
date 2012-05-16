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

#include "FCVoice.h"
#include "../flod_internal.h"

void FCVoice_defaults(struct FCVoice* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

void FCVoice_ctor(struct FCVoice* self) {
	CLASS_CTOR_DEF(FCVoice);
	// original constructor code goes here
	self->index = index;
}

struct FCVoice* FCVoice_new(int index) {
	CLASS_NEW_BODY(FCVoice, index);
}

void FCVoice_initialize(struct FCVoice* self) {
	CLASS_DEF_INIT(); // zeromem
	self->volCtr         = 1;
	self->volSpeed       = 1;
}

void FCVoice_volumeBend(struct FCVoice* self) {
	self->volBendFlag ^= 1;

	if (self->volBendFlag) {
		self->volBendTime--;
		self->volume += self->volBendSpeed;
		if (self->volume < 0 || self->volume > 64) self->volBendTime = 0;
	}
}
