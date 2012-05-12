/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.1 - 2012/04/21

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/
package neoart->flod->core {

  public class SBPlayer extends CorePlayer {
    public var
      mixer   : Soundblaster,
 int length;
 int restart;
      track   : Vector.<int>;
    protected var
 int timer;
 int master;

     void SBPlayer(mixer:Soundblaster = null) {
      this->mixer = mixer || new Soundblaster();
      super(this->mixer);

      endian  = "littleEndian";
      quality = 1;
  }

//override
void set volume( Number value) {
      if (value < 0.0) value = 0.0;
        else if (value > 1.0) value = 1.0;

      master = value * 64;
    }

//override
void toggle( int index) {
      mixer->channels[index].mute ^= 1;
    }

//override
void setup() {
      mixer->setup(channels);
    }

//override
void initialize() {
      super->initialize();
      timer  = speed;
      master = 64;
    }
  }
}