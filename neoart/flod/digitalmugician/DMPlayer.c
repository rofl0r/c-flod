/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.0 - 2012/03/10

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "DMPlayer.h"
#include "../flod_internal.h"
#include "DMPlayer_const.h"

void DMPlayer_defaults(struct DMPlayer* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

// amiga default null
void DMPlayer_ctor(struct DMPlayer* self, struct Amiga *amiga) {
	CLASS_CTOR_DEF(DMPlayer);
	// original constructor code goes here
	super(amiga);
	PERIODS->fixed = true;

	songs     = new Vector.<DMSong>(8, true);
	arpeggios = new Vector.<int>(256, true);
	voices    = new Vector.<DMVoice>(7, true);

	voices[0] = new DMVoice();
	voices[1] = new DMVoice();
	voices[2] = new DMVoice();
	voices[3] = new DMVoice();
	voices[4] = new DMVoice();
	voices[5] = new DMVoice();
	voices[6] = new DMVoice();
	tables();
	
}

struct DMPlayer* DMPlayer_new(struct Amiga *amiga) {
	CLASS_NEW_BODY(DMPlayer, amiga);
}

//override
void DMPlayer_process() {
	struct AmigaChannel *chan = 0;
	int dst = 0;
	int i = 0; 
	int idx = 0; 
	int j = 0; 
	int len = 0; 
	//memory:Vector.<int> = amiga->memory;
	int r = 0; 
	struct AmigaRow *row = 0;
	int src1 = 0;
	int src2 = 0;
	struct DMSample *sample = 0;
	int value = 0;
	struct DMVoice *voice = 0;

	for (i = 0; i < numChannels; ++i) {
		voice = voices[i];
		sample = voice->sample;

		if (i < 3 || numChannels == 4) {
			chan = voice->channel;
			if (stepEnd) voice->step = song1.tracks[int(trackPos + i)];

			if (sample->wave > 31) {
				chan->pointer = sample->loopPtr;
				chan->length  = sample->repeat;
			}
		} else {
			chan = mixChannel;
			if (stepEnd) voice->step = song2.tracks[int(trackPos + (i - 3))];
		}

		if (patternEnd) {
			row = patterns[int(voice->step->pattern + patternPos)];

			if (row->note) {
				if (row->effect != 74) {
					voice->note = row->note;
					if (row->sample) sample = voice->sample = samples[row->sample];
				}
				voice->val1 = row->effect < 64 ? 1 : row->effect - 62;
				voice->val2 = row->param;
				idx = voice->step->transpose + sample->finetune;

				if (voice->val1 != 12) {
					voice->pitch = row->effect;

					if (voice->val1 == 1) {
						idx += voice->pitch;
						if (idx < 0) voice->period = 0;
						else voice->period = PERIODS[idx];
					}
				} else {
					voice->pitch = row->note;
					idx += voice->pitch;

					if (idx < 0) voice->period = 0;
						else voice->period = PERIODS[idx];
				}

				if (voice->val1 == 11) sample->arpeggio = voice->val2 & 7;

				if (voice->val1 != 12) {
					if (sample->wave > 31) {
						chan->pointer  = sample->pointer;
						chan->length   = sample->length;
						chan->enabled  = 0;
						voice->mixPtr  = sample->pointer;
						voice->mixEnd  = sample->pointer + sample->length;
						voice->mixMute = 0;
					} else {
						dst = sample->wave << 7;
						chan->pointer = dst;
						chan->length  = sample->waveLen;
						if (voice->val1 != 10) chan->enabled = 0;

						if (numChannels == 4) {
							if (sample->effect != 0 && voice->val1 != 2 && voice->val1 != 4) {
								len  = dst + 128;
								src1 = sample->source1 << 7;
								for (j = dst; j < len; ++j) memory[j] = memory[src1++];

								sample->effectStep = 0;
								voice->effectCtr   = sample->effectSpeed;
							}
						}
					}
				}

				if (voice->val1 != 3 && voice->val1 != 4 && voice->val1 != 12) {
					voice->volumeCtr  = 1;
					voice->volumeStep = 0;
				}

				voice->arpeggioStep = 0;
				voice->pitchCtr     = sample->pitchDelay;
				voice->pitchStep    = 0;
				voice->portamento   = 0;
			}
		}

		switch (voice->val1) {
			case 0:
				break;
			case 5:   //pattern length
				value = voice->val2;
				if (value > 0 && value < 65) patternLen = value;
				break;
			case 6:   //song speed
				value  = voice->val2 & 15;
				value |= value << 4;
				if (voice->val2 == 0 || voice->val2 > 15) break;
				speed = value;
				break;
			case 7:   //led filter on
				amiga->filter->active = 1;
				break;
			case 8:   //led filter off
				amiga->filter->active = 0;
				break;
			case 13:  //shuffle
				voice->val1 = 0;
				value = voice->val2 & 0x0f;
				if (value == 0) break;
				value = voice->val2 & 0xf0;
				if (value == 0) break;
				speed = voice->val2;
				break;
			default:
				break;
		}
	}

	for (i = 0; i < numChannels; ++i) {
		voice  = voices[i];
		sample = voice->sample;

		if (numChannels == 4) {
			chan = voice->channel;

			if (sample->wave < 32 && sample->effect && !sample->effectDone) {
				sample->effectDone = 1;

				if (voice->effectCtr) {
					voice->effectCtr--;
				} else {
					voice->effectCtr = sample->effectSpeed;
					dst = sample->wave << 7;

					switch (sample->effect) {
						case 1:   //filter
							for (j = 0; j < 127; ++j) {
								value  = memory[dst];
								value += memory[int(dst + 1)];
								memory[dst++] = value >> 1;
							}
							break;
						case 2:   //mixing
							src1 = sample->source1 << 7;
							src2 = sample->source2 << 7;
							idx  = sample->effectStep;
							len  = sample->waveLen;
							sample->effectStep = ++sample->effectStep & 127;

							for (j = 0; j < len; ++j) {
								value  = memory[src1++];
								value += memory[int(src2 + idx)];
								memory[dst++] = value >> 1;
								idx = ++idx & 127;
							}
							break;
						case 3:   //scr left
							value = memory[dst];
							for (j = 0; j < 127; ++j) memory[dst] = memory[++dst];
							memory[dst] = value;
							break;
						case 4:   //scr right
							dst += 127;
							value = memory[dst];
							for (j = 0; j < 127; ++j) memory[dst] = memory[--dst];
							memory[dst] = value;
							break;
						case 5:   //upsample
							idx = value = dst;
							for (j = 0; j < 64; ++j) {
								memory[idx++] = memory[dst++];
								dst++;
							}
							idx = dst = value;
							idx += 64;
							for (j = 0; j < 64; ++j) memory[idx++] = memory[dst++];
							break;
						case 6:   //downsample
							src1 = dst + 64;
							dst += 128;
							for (j = 0; j < 64; ++j) {
								memory[--dst] = memory[--src1];
								memory[--dst] = memory[src1];
							}
							break;
						case 7:   //negate
							dst += sample->effectStep;
							memory[dst] = ~memory[dst] + 1;
							if (++sample->effectStep >= sample->waveLen) sample->effectStep = 0;
							break;
						case 8:   //madmix 1
							sample->effectStep = ++sample->effectStep & 127;
							src2 = (sample->source2 << 7) + sample->effectStep;
							idx  = memory[src2];
							len  = sample->waveLen;
							value = 3;

							for (j = 0; j < len; ++j) {
								src1 = memory[dst] + value;
								if (src1 < -128) src1 += 256;
								else if (src1 > 127) src1 -= 256;

								memory[dst++] = src1;
								value += idx;

								if (value < -128) value += 256;
								else if (value > 127) value -= 256;
							}
							break;
						case 9:   //addition
							src2 = sample->source2 << 7;
							len  = sample->waveLen;

							for (j = 0; j < len; ++j) {
								value  = memory[src2++];
								value += memory[dst];
								if (value > 127) value -= 256;
								memory[dst++] = value;
							}
							break;
						case 10:  //filter 2
							for (j = 0; j < 126; ++j) {
								value  = memory[dst++] * 3;
								value += memory[int(dst + 1)];
								memory[dst] = value >> 2;
							}
							break;
						case 11:  //morphing
							src1 = sample->source1 << 7;
							src2 = sample->source2 << 7;
							len  = sample->waveLen;

							sample->effectStep = ++sample->effectStep & 127;
							value = sample->effectStep;
							if (value >= 64) value = 127 - value;
							idx = (value ^ 255) & 63;

							for (j = 0; j < len; ++j) {
								r  = memory[src1++] * value;
								r += memory[src2++] * idx;
								memory[dst++] = r >> 6;
							}
							break;
						case 12:  //morph f
							src1 = sample->source1 << 7;
							src2 = sample->source2 << 7;
							len  = sample->waveLen;

							sample->effectStep = ++sample->effectStep & 31;
							value = sample->effectStep;
							if (value >= 16) value = 31 - value;
							idx = (value ^ 255) & 15;

							for (j = 0; j < len; ++j) {
								r  = memory[src1++] * value;
								r += memory[src2++] * idx;
								memory[dst++] = r >> 4;
							}
							break;
						case 13:  //filter 3
							for (j = 0; j < 126; ++j) {
								value  = memory[dst++];
								value += memory[int(dst + 1)];
								memory[dst] = value >> 1;
							}
							break;
						case 14:  //polygate
							idx = dst + sample->effectStep;
							memory[idx] = ~memory[idx] + 1;
							idx = (sample->effectStep + sample->source2) & (sample->waveLen - 1);
							idx += dst;
							memory[idx] = ~memory[idx] + 1;
							if (++sample->effectStep >= sample->waveLen) sample->effectStep = 0;
							break;
						case 15:  //colgate
							idx = dst;
							for (j = 0; j < 127; ++j) {
								value  = memory[dst];
								value += memory[int(dst + 1)];
								memory[dst++] = value >> 1;
							}
							dst = idx;
							sample->effectStep++;

							if (sample->effectStep == sample->source2) {
								sample->effectStep = 0;
								idx = value = dst;

								for (j = 0; j < 64; ++j) {
									memory[idx++] = memory[dst++];
									dst++;
								}
								idx = dst = value;
								idx += 64;
								for (j = 0; j < 64; ++j) memory[idx++] = memory[dst++];
							}
							break;
						default:
							break;
					}
				}
			}
		} else {
			chan = (i < 3) ? voice->channel : self->mixChannel;
		}

		if (voice->volumeCtr) {
			voice->volumeCtr--;

			if (voice->volumeCtr == 0) {
				voice->volumeCtr  = sample->volumeSpeed;
				voice->volumeStep = ++voice->volumeStep & 127;

				if (voice->volumeStep || sample->volumeLoop) {
					idx = voice->volumeStep + (sample->volume << 7);
					value = ~(memory[idx] + 129) + 1;

					voice->volume = (value & 255) >> 2;
					chan->volume  = voice->volume;
				} else {
					voice->volumeCtr = 0;
				}
			}
		}
		value = voice->note;

		if (sample->arpeggio) {
			idx = voice->arpeggioStep + (sample->arpeggio << 5);
			value += arpeggios[idx];
			voice->arpeggioStep = ++voice->arpeggioStep & 31;
		}

		idx = value + voice->step->transpose + sample->finetune;
		voice->finalPeriod = PERIODS[idx];
		dst = voice->finalPeriod;

		if (voice->val1 == 1 || voice->val1 == 12) {
			value = ~voice->val2 + 1;
			voice->portamento += value;
			voice->finalPeriod += voice->portamento;

			if (voice->val2) {
				if ((value < 0 && voice->finalPeriod <= voice->period) || (value >= 0 && voice->finalPeriod >= voice->period)) {
					voice->portamento = voice->period - dst;
					voice->val2 = 0;
				}
			}
		}

		if (sample->pitch) {
			if (voice->pitchCtr) {
				voice->pitchCtr--;
			} else {
				idx = voice->pitchStep;
				voice->pitchStep = ++voice->pitchStep & 127;
				if (voice->pitchStep == 0) voice->pitchStep = sample->pitchLoop;

				idx += sample->pitch << 7;
				value = memory[idx];
				voice->finalPeriod += (~value + 1);
			}
		}
		chan->period = voice->finalPeriod;
	}

	if (numChannels > 4) {
		src1 = buffer1;
		buffer1 = buffer2;
		buffer2 = src1;

		chan = amiga->channels[3];
		chan->pointer = src1;

		for (i = 3; i < 7; ++i) {
			voice = voices[i];
			voice->mixStep = 0;

			if (voice->finalPeriod < 125) {
				voice->mixMute  = 1;
				voice->mixSpeed = 0;
			} else {
				j = ((voice->finalPeriod << 8) / mixPeriod) & 65535;
				src2 = ((256 / j) & 255) << 8;
				dst  = ((256 % j) << 8) & 16777215;
				voice->mixSpeed = (src2 | ((dst / j) & 255)) << 8;
			}

			if (voice->mixMute) voice->mixVolume = 0;
			else voice->mixVolume = voice->volume << 8;
		}

		for (i = 0; i < 350; ++i) {
			dst = 0;

			for (j = 3; j < 7; ++j) {
				voice = voices[j];
				src2 = (memory[int(voice->mixPtr + (voice->mixStep >> 16))] & 255) + voice->mixVolume;
				dst += volumes[src2];
				voice->mixStep += voice->mixSpeed;
			}

			memory[src1++] = averages[dst];
		}
		chan->length = 350;
		chan->period = mixPeriod;
		chan->volume = 64;
	}

	if (--tick == 0) {
		tick = speed & 15;
		speed  = (speed & 240) >> 4;
		speed |= (tick << 4);
		patternEnd = 1;
		patternPos++;

		if (patternPos == 64 || patternPos == patternLen) {
			patternPos = 0;
			stepEnd    = 1;
			trackPos  += 4;

			if (trackPos == song1.length) {
				trackPos = song1.loopStep;
				amiga->complete = 1;
			}
		}
	} else {
		patternEnd = 0;
		stepEnd = 0;
	}

	for (i = 0; i < numChannels; ++i) {
		voice = voices[i];
		voice->mixPtr += voice->mixStep >> 16;

		sample = voice->sample;
		sample->effectDone = 0;

		if (voice->mixPtr >= voice->mixEnd) {
			if (sample->loop) {
				voice->mixPtr -= sample->repeat;
			} else {
				voice->mixPtr  = 0;
				voice->mixMute = 1;
			}
		}

		if (i < 4) {
			chan = voice->channel;
			chan->enabled = 1;
		}
	}
}

//override
void DMPlayer_initialize() {
	struct AmigaChannel *chan = 0;
	int i = 0;
	int len = 0;
	struct DMVoice *voice = 0;
	
	super->initialize();

	if (playSong > 7) playSong = 0;

	song1  = songs[playSong];
	speed  = song1.speed & 0x0f;
	speed |= speed << 4;
	tick   = song1.speed;

	trackPos    = 0;
	patternPos  = 0;
	patternLen  = 64;
	patternEnd  = 1;
	stepEnd     = 1;
	numChannels = 4;

	for (; i < 7; ++i) {
		voice = voices[i];
		voice->initialize();
		voice->sample = samples[0];

		if (i < 4) {
			chan = amiga->channels[i];
			chan->enabled = 0;
			chan->pointer = amiga->loopPtr;
			chan->length  = 2;
			chan->period  = 124;
			chan->volume  = 0;

			voice->channel = chan;
		}
	}

	if (version == DIGITALMUG_V2) {
		if ((playSong & 1) != 0) playSong--;
		song2 = songs[int(playSong + 1)];

		mixChannel  = new AmigaChannel(7);
		numChannels = 7;

		chan = amiga->channels[3];
		chan->mute    = 0;
		chan->pointer = buffer1;
		chan->length  = 350;
		chan->period  = mixPeriod;
		chan->volume  = 64;

		len = buffer1 + 700;
		for (i = buffer1; i < len; ++i) amiga->memory[i] = 0;
	}
}

//override
void DMPlayer_loader(stream:ByteArray) {
	int data = 0; 
	int i = 0; 
	char id[28];
	//index:Vector.<int>;
	int *index = 0;
	int instr = 0; 
	int j = 0; 
	int len = 0; 
	int position = 0; 
	struct AmigaRow *row = 0;
	struct DMSample *sample = 0;
	struct DMSong *song = 0;
	struct AmigaStep *step = 0;
	
	id = stream->readMultiByte(24, ENCODING);

	if (id == " MUGICIAN/SOFTEYES 1990 ") version = DIGITALMUG_V1;
	else if (id == " MUGICIAN2/SOFTEYES 1990") version = DIGITALMUG_V2;
	else return;

	stream->position = 28;
	index = new Vector.<int>(8, true);
	for (; i < 8; ++i) index[i] = stream->readUnsignedInt();

	stream->position = 76;

	for (i = 0; i < 8; ++i) {
		song = new DMSong();
		song->loop     = stream->readUnsignedByte();
		song->loopStep = stream->readUnsignedByte() << 2;
		song->speed    = stream->readUnsignedByte();
		song->length   = stream->readUnsignedByte() << 2;
		song->title    = stream->readMultiByte(12, ENCODING);
		songs[i] = song;
	}

	stream->position = 204;
	lastSong = songs->length - 1;

	for (i = 0; i < 8; ++i) {
		song = songs[i];
		len  = index[i] << 2;

		for (j = 0; j < len; ++j) {
			step = new AmigaStep();
			step->pattern   = stream->readUnsignedByte() << 6;
			step->transpose = stream->readByte();
			song->tracks[j] = step;
		}
		song->tracks->fixed = true;
	}

	position = stream->position;
	stream->position = 60;
	len = stream->readUnsignedInt();
	samples = new Vector.<DMSample>(++len, true);
	stream->position = position;

	for (i = 1; i < len; ++i) {
		sample = new DMSample();
		sample->wave        = stream->readUnsignedByte();
		sample->waveLen     = stream->readUnsignedByte() << 1;
		sample->volume      = stream->readUnsignedByte();
		sample->volumeSpeed = stream->readUnsignedByte();
		sample->arpeggio    = stream->readUnsignedByte();
		sample->pitch       = stream->readUnsignedByte();
		sample->effectStep  = stream->readUnsignedByte();
		sample->pitchDelay  = stream->readUnsignedByte();
		sample->finetune    = stream->readUnsignedByte() << 6;
		sample->pitchLoop   = stream->readUnsignedByte();
		sample->pitchSpeed  = stream->readUnsignedByte();
		sample->effect      = stream->readUnsignedByte();
		sample->source1     = stream->readUnsignedByte();
		sample->source2     = stream->readUnsignedByte();
		sample->effectSpeed = stream->readUnsignedByte();
		sample->volumeLoop  = stream->readUnsignedByte();
		samples[i] = sample;
	}
	samples[0] = samples[1];

	position = stream->position;
	stream->position = 64;
	len = stream->readUnsignedInt() << 7;
	stream->position = position;
	amiga->store(stream, len);

	position = stream->position;
	stream->position = 68;
	instr = stream->readUnsignedInt();

	stream->position = 26;
	len = stream->readUnsignedShort() << 6;
	patterns = new Vector.<AmigaRow>(len, true);
	stream->position = position + (instr << 5);

	if (instr) instr = position;

	for (i = 0; i < len; ++i) {
		row = new AmigaRow();
		row->note   = stream->readUnsignedByte();
		row->sample = stream->readUnsignedByte() & 63;
		row->effect = stream->readUnsignedByte();
		row->param  = stream->readByte();
		patterns[i] = row;
	}

	position = stream->position;
	stream->position = 72;

	if (instr) {
		len = stream->readUnsignedInt();
		stream->position = position;
		data = amiga->store(stream, len);
		position = stream->position;

		amiga->memory->length += 350;
		buffer1 = amiga->memory->length;
		amiga->memory->length += 350;
		buffer2 = amiga->memory->length;
		amiga->memory->length += 350;
		amiga->loopLen = 8;

		len = samples->length;

		for (i = 1; i < len; ++i) {
			sample = samples[i];
			if (sample->wave < 32) continue;
			stream->position = instr + ((sample->wave - 32) << 5);

			sample->pointer = stream->readUnsignedInt();
			sample->length  = stream->readUnsignedInt() - sample->pointer;
			sample->loop    = stream->readUnsignedInt();
			sample->name    = stream->readMultiByte(12, ENCODING);

			if (sample->loop) {
				sample->loop  -= sample->pointer;
				sample->repeat = sample->length - sample->loop;
				if ((sample->repeat & 1) != 0) sample->repeat--;
			} else {
				sample->loopPtr = amiga->memory->length;
				sample->repeat  = 8;
			}

			if ((sample->pointer & 1) != 0) sample->pointer--;
			if ((sample->length  & 1) != 0) sample->length--;

			sample->pointer += data;
			if (!sample->loopPtr) sample->loopPtr = sample->pointer + sample->loop;
		}
	} else {
		position += stream->readUnsignedInt();
	}
	stream->position = 24;

	if (stream->readUnsignedShort() == 1) {
		stream->position = position;
		len = stream->length - stream->position;
		if (len > 256) len = 256;
		for (i = 0; i < len; ++i) arpeggios[i] = stream->readUnsignedByte();
	}
}

void tables() {
	var int i = 0;
	int idx = 0;
	int j = 0; 
	int pos = 0; 
	int step = 0; 
	int v1 = 0; 
	int v2 = 0; 
	int vol = 128;

	averages  = new Vector.<int>(1024, true);
	volumes   = new Vector.<int>(16384, true);
	mixPeriod = 203;

	for (; i < 1024; ++i) {
		if (vol > 127) vol -= 256;
		averages[i] = vol;
		if (i > 383 && i < 639) vol = ++vol & 255;
	}

	for (i = 0; i < 64; ++i) {
		v1 = -128;
		v2 =  128;

		for (j = 0; j < 256; ++j) {
			vol = ((v1 * step) / 63) + 128;
			idx = pos + v2;
			volumes[idx] = vol & 255;

			if (i != 0 && i != 63 && v2 >= 128) --volumes[idx];
			v1++;
			v2 = ++v2 & 255;
		}
		pos += 256;
		step++;
	}
}


