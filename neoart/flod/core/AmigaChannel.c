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
package neoart->flod->core {

  public final class AmigaChannel {
    public var
      next    : AmigaChannel,
 int mute;
      panning : Number = 1.0,
 int delay;
 int pointer;
 int length;
    internal var
 int audena;
 int audcnt;
 int audloc;
 int audper;
 int audvol;
      timer   : Number,
      level   : Number,
      ldata   : Number,
      rdata   : Number;

     void AmigaChannel(index:int) {
      if ((++index & 2) == 0) panning = -panning;
      level = panning;
    }

     void get enabled():int { return audena; }

     void set enabled(value:int):void {
      if (value == audena) return;

      audena = value;
      audloc = pointer;
      audcnt = pointer + length;

      timer = 1.0;
      if (value) delay += 2;
    }

     void set period(value:int):void {
      if (value < 0) value = 0;
        else if(value > 65535) value = 65535;

      audper = value;
    }

     void set volume(value:int):void {
      if (value < 0) value = 0;
        else if (value > 64) value = 64;

      audvol = value;
    }

     void resetData():void {
      ldata = 0.0;
      rdata = 0.0;
    }

    internal function initialize():void {
      audena = 0;
      audcnt = 0;
      audloc = 0;
      audper = 50;
      audvol = 0;

      timer = 0.0;
      ldata = 0.0;
      rdata = 0.0;

      delay   = 0;
      pointer = 0;
      length  = 0;
    }
  }
}