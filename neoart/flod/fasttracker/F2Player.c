/*
  Flod 4.1
  2012/04/30
  Christian Corti
  Neoart Costa Rica

  Last Update: Flod 4.0 - 2012/03/05

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 Unported License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
  Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

#include "F2Player.h"
#include "../flod_internal.h"
#include "F2Player_const.h"

void F2Player_defaults(struct F2Player* self) {
	CLASS_DEF_INIT();
	// static initializers go here
}

/* mixer default value = null */
void F2Player_ctor(struct F2Player* self, struct Soundblaster *mixer) {
	CLASS_CTOR_DEF(F2Player);
	// original constructor code goes here
	super(mixer);
}

struct F2Player* F2Player_new(struct Soundblaster *mixer) {
	CLASS_NEW_BODY(F2Player, mixer);
}

//override
void F2Player_process(struct F2Player* self) {
      int com = 0;
      struct F2Point *curr = NULL;
      struct F2Instrument *instr = NULL;
      int i = 0;
      int jumpFlag = 0;
      struct F2Point *next = NULL;
      int paramx = 0;
      int paramy = 0;
      int porta = 0;
      struct F2Row *row = NULL;
      struct F2Sample *sample = NULL;
      int slide = 0;
      int value = 0;
      struct F2Voice *voice = voices[0];

      if (!tick) {
        if (nextOrder >= 0) order = nextOrder;
        if (nextPosition >= 0) position = nextPosition;

        nextOrder = nextPosition = -1;
        pattern = patterns[track[order]];

        while (voice) {
          row = pattern->rows[int(position + voice->index)];
          com = row->volume >> 4;
          porta = int(row->effect == FX_TONE_PORTAMENTO || row->effect == FX_TONE_PORTA_VOLUME_SLIDE || com == VX_TONE_PORTAMENTO);
          paramx = row->param >> 4;
          voice->keyoff = 0;

          if (voice->arpDelta) {
            voice->arpDelta = 0;
            voice->flags |= UPDATE_PERIOD;
          }

          if (row->instrument) {
            voice->instrument = row->instrument < instruments->length ? instruments[row->instrument] : null;
            voice->volEnvelope->reset();
            voice->panEnvelope->reset();
            voice->flags |= (UPDATE_VOLUME | UPDATE_PANNING | SHORT_RAMP);
          } else if (row->note == KEYOFF_NOTE || (row->effect == FX_KEYOFF && !row->param)) {
            voice->fadeEnabled = 1;
            voice->keyoff = 1;
          }

          if (row->note && row->note != KEYOFF_NOTE) {
            if (voice->instrument) {
              instr  = voice->instrument;
              value  = row->note - 1;
              sample = instr->samples[instr->noteSamples[value]];
              value += sample->relative;

              if (value >= LOWER_NOTE && value <= HIGHER_NOTE) {
                if (!porta) {
                  voice->note = value;
                  voice->sample = sample;

                  if (row->instrument) {
                    voice->volEnabled = instr->volEnabled;
                    voice->panEnabled = instr->panEnabled;
                    voice->flags |= UPDATE_ALL;
                  } else {
                    voice->flags |= (UPDATE_PERIOD | UPDATE_TRIGGER);
                  }
                }

                if (row->instrument) {
                  voice->reset();
                  voice->fadeDelta = instr->fadeout;
                } else {
                  voice->finetune = (sample->finetune >> 3) << 2;
                }

                if (row->effect == FX_EXTENDED_EFFECTS && paramx == EX_SET_FINETUNE)
                  voice->finetune = ((row->param & 15) - 8) << 3;

                if (linear) {
                  value = ((120 - value) << 6) - voice->finetune;
                } else {
                  value = amiga(value, voice->finetune);
                }

                if (!porta) {
                  voice->period = value;
                  voice->glissPeriod = 0;
                } else {
                  voice->portaPeriod = value;
                }
              }
            } else {
              voice->volume = 0;
              voice->flags = (UPDATE_VOLUME | SHORT_RAMP);
            }
          } else if (voice->vibratoReset) {
            if (row->effect != FX_VIBRATO && row->effect != FX_VIBRATO_VOLUME_SLIDE) {
              voice->vibDelta = 0;
              voice->vibratoReset = 0;
              voice->flags |= UPDATE_PERIOD;
            }
          }

          if (row->volume) {
            if (row->volume >= 16 && row->volume <= 80) {
              voice->volume = row->volume - 16;
              voice->flags |= (UPDATE_VOLUME | SHORT_RAMP);
            } else {
              paramy = row->volume & 15;

              switch (com) {
                case VX_FINE_VOLUME_SLIDE_DOWN:
                  voice->volume -= paramy;
                  if (voice->volume < 0) voice->volume = 0;
                  voice->flags |= UPDATE_VOLUME;
                  break;
                case VX_FINE_VOLUME_SLIDE_UP:
                  voice->volume += paramy;
                  if (voice->volume > 64) voice->volume = 64;
                  voice->flags |= UPDATE_VOLUME;
                  break;
                case VX_SET_VIBRATO_SPEED:
                  if (paramy) voice->vibratoSpeed = paramy;
                  break;
                case VX_VIBRATO:
                  if (paramy) voice->vibratoDepth = paramy << 2;
                  break;
                case VX_SET_PANNING:
                  voice->panning = paramy << 4;
                  voice->flags |= UPDATE_PANNING;
                  break;
                case VX_TONE_PORTAMENTO:
                  if (paramy) voice->portaSpeed = paramy << 4;
                  break;
              }
            }
          }

          if (row->effect) {
            paramy = row->param & 15;

            switch (row->effect) {
              case FX_PORTAMENTO_UP:
                if (row->param) voice->portaU = row->param << 2;
                break;
              case FX_PORTAMENTO_DOWN:
                if (row->param) voice->portaD = row->param << 2;
                break;
              case FX_TONE_PORTAMENTO:
                if (row->param && com != VX_TONE_PORTAMENTO) voice->portaSpeed = row->param;
                break;
              case FX_VIBRATO:
                voice->vibratoReset = 1;
                break;
              case FX_TONE_PORTA_VOLUME_SLIDE:
                if (row->param) voice->volSlide = row->param;
                break;
              case FX_VIBRATO_VOLUME_SLIDE:
                if (row->param) voice->volSlide = row->param;
                voice->vibratoReset = 1;
                break;
              case FX_TREMOLO:
                if (paramx) voice->tremoloSpeed = paramx;
                if (paramy) voice->tremoloDepth = paramy;
                break;
              case FX_SET_PANNING:
                voice->panning = row->param;
                voice->flags |= UPDATE_PANNING;
                break;
              case FX_SAMPLE_OFFSET:
                if (row->param) voice->sampleOffset = row->param << 8;

                if (voice->sampleOffset >= voice->sample->length) {
                  voice->volume = 0;
                  voice->sampleOffset = 0;
                  voice->flags &= ~(UPDATE_PERIOD | UPDATE_TRIGGER);
                  voice->flags |=  (UPDATE_VOLUME | SHORT_RAMP);
                }
                break;
              case FX_VOLUME_SLIDE:
                if (row->param) voice->volSlide = row->param;
                break;
              case FX_POSITION_JUMP:
                nextOrder = row->param;

                if (nextOrder >= length) complete = 1;
                  else nextPosition = 0;

                jumpFlag      = 1;
                patternOffset = 0;
                break;
              case FX_SET_VOLUME:
                voice->volume = row->param;
                voice->flags |= (UPDATE_VOLUME | SHORT_RAMP);
                break;
              case FX_PATTERN_BREAK:
                nextPosition  = ((paramx * 10) + paramy) * channels;
                patternOffset = 0;

                if (!jumpFlag) {
                  nextOrder = order + 1;

                  if (nextOrder >= length) {
                    complete = 1;
                    nextPosition = -1;
                  }
                }
                break;
              case FX_EXTENDED_EFFECTS:

                switch (paramx) {
                  case EX_FINE_PORTAMENTO_UP:
                    if (paramy) voice->finePortaU = paramy << 2;
                    voice->period -= voice->finePortaU;
                    voice->flags |= UPDATE_PERIOD;
                    break;
                  case EX_FINE_PORTAMENTO_DOWN:
                    if (paramy) voice->finePortaD = paramy << 2;
                    voice->period += voice->finePortaD;
                    voice->flags |= UPDATE_PERIOD;
                    break;
                  case EX_GLISSANDO_CONTROL:
                    voice->glissando = paramy;
                    break;
                  case EX_VIBRATO_CONTROL:
                    voice->waveControl = (voice->waveControl & 0xf0) | paramy;
                    break;
                  case EX_PATTERN_LOOP:
                    if (!paramy) {
                      voice->patternLoopRow = patternOffset = position;
                    } else {
                      if (!voice->patternLoop) {
                        voice->patternLoop = paramy;
                      } else {
                        voice->patternLoop--;
                      }

                      if (voice->patternLoop)
                        nextPosition = voice->patternLoopRow;
                    }
                    break;
                  case EX_TREMOLO_CONTROL:
                    voice->waveControl = (voice->waveControl & 0x0f) | (paramy << 4);
                    break;
                  case EX_FINE_VOLUME_SLIDE_UP:
                    if (paramy) voice->fineSlideU = paramy;
                    voice->volume += voice->fineSlideU;
                    voice->flags |= UPDATE_VOLUME;
                    break;
                  case EX_FINE_VOLUME_SLIDE_DOWN:
                    if (paramy) voice->fineSlideD = paramy;
                    voice->volume -= voice->fineSlideD;
                    voice->flags |= UPDATE_VOLUME;
                    break;
                  case EX_NOTE_DELAY:
                    voice->delay = voice->flags;
                    voice->flags = 0;
                    break;
                  case EX_PATTERN_DELAY:
                    patternDelay = paramy * timer;
                    break;
                }

                break;
              case FX_SET_SPEED:
                if (!row->param) break;
                if (row->param < 32) timer = row->param;
                  else mixer->samplesTick = 110250 / row->param;
                break;
              case FX_SET_GLOBAL_VOLUME:
                master = row->param;
                if (master > 64) master = 64;
                voice->flags |= UPDATE_VOLUME;
                break;
              case FX_GLOBAL_VOLUME_SLIDE:
                if (row->param) voice->volSlideMaster = row->param;
                break;
              case FX_SET_ENVELOPE_POSITION:
                if (!voice->instrument || !voice->instrument->volEnabled) break;
                instr  = voice->instrument;
                value  = row->param;
                paramx = instr->volData->total;

                for (i = 0; i < paramx; ++i)
                  if (value < instr->volData->points[i].frame) break;

                voice->volEnvelope->position = --i;
                paramx--;

                if ((instr->volData->flags & ENVELOPE_LOOP) && i == instr->volData->loopEnd) {
                  i = voice->volEnvelope->position = instr->volData->loopStart;
                  value = instr->volData->points[i].frame;
                  voice->volEnvelope->frame = value;
                }

                if (i >= paramx) {
                  voice->volEnvelope->value = instr->volData->points[paramx].value;
                  voice->volEnvelope->stopped = 1;
                } else {
                  voice->volEnvelope->stopped = 0;
                  voice->volEnvelope->frame = value;
                  if (value > instr->volData->points[i].frame) voice->volEnvelope->position++;

                  curr = instr->volData->points[i];
                  next = instr->volData->points[++i];
                  value = next->frame - curr->frame;

                  voice->volEnvelope->delta = value ? ((next->value - curr->value) << 8) / value : 0;
                  voice->volEnvelope->fraction = (curr->value << 8);
                }
                break;
              case FX_PANNING_SLIDE:
                if (row->param) voice->panSlide = row->param;
                break;
              case FX_MULTI_RETRIG_NOTE:
                if (paramx) voice->retrigx = paramx;
                if (paramy) voice->retrigy = paramy;

                if (!row->volume && voice->retrigy) {
                  com = tick + 1;
                  if (com % voice->retrigy) break;
                  if (row->volume > 80 && voice->retrigx) retrig(voice);
                }
                break;
              case FX_TREMOR:
                if (row->param) {
                  voice->tremorOn  = ++paramx;
                  voice->tremorOff = ++paramy + paramx;
                }
                break;
              case FX_EXTRA_FINE_PORTAMENTO:
                if (paramx == 1) {
                  if (paramy) voice->xtraPortaU = paramy;
                  voice->period -= voice->xtraPortaU;
                  voice->flags |= UPDATE_PERIOD;
                } else if (paramx == 2) {
                  if (paramy) voice->xtraPortaD = paramy;
                  voice->period += voice->xtraPortaD;
                  voice->flags |= UPDATE_PERIOD;
                }
                break;
            }
          }
          voice = voice->next;
        }
      } else {
        while (voice) {
          row = pattern->rows[int(position + voice->index)];

          if (voice->delay) {
            if ((row->param & 15) == tick) {
              voice->flags = voice->delay;
              voice->delay = 0;
            } else {
              voice = voice->next;
              continue;
            }
          }

          if (row->volume) {
            paramx = row->volume >> 4;
            paramy = row->volume & 15;

            switch (paramx) {
              case VX_VOLUME_SLIDE_DOWN:
                voice->volume -= paramy;
                if (voice->volume < 0) voice->volume = 0;
                voice->flags |= UPDATE_VOLUME;
                break;
              case VX_VOLUME_SLIDE_UP:
                voice->volume += paramy;
                if (voice->volume > 64) voice->volume = 64;
                voice->flags |= UPDATE_VOLUME;
                break;
              case VX_VIBRATO:
                voice->vibrato();
                break;
              case VX_PANNING_SLIDE_LEFT:
                voice->panning -= paramy;
                if (voice->panning < 0) voice->panning = 0;
                voice->flags |= UPDATE_PANNING;
                break;
              case VX_PANNING_SLIDE_RIGHT:
                voice->panning += paramy;
                if (voice->panning > 255) voice->panning = 255;
                voice->flags |= UPDATE_PANNING;
                break;
              case VX_TONE_PORTAMENTO:
                if (voice->portaPeriod) voice->tonePortamento();
                break;
            }
          }

          paramx = row->param >> 4;
          paramy = row->param & 15;

          switch (row->effect) {
            case FX_ARPEGGIO:
              if (!row->param) break;
              value = (tick - timer) % 3;
              if (value < 0) value += 3;
              if (tick == 2 && timer == 18) value = 0;

              if (!value) {
                voice->arpDelta = 0;
              } else if (value == 1) {
                if (linear) {
                  voice->arpDelta = -(paramy << 6);
                } else {
                  value = amiga(voice->note + paramy, voice->finetune);
                  voice->arpDelta = value - voice->period;
                }
              } else {
                if (linear) {
                  voice->arpDelta = -(paramx << 6);
                } else {
                  value = amiga(voice->note + paramx, voice->finetune);
                  voice->arpDelta = value - voice->period;
                }
              }

              voice->flags |= UPDATE_PERIOD;
              break;
            case FX_PORTAMENTO_UP:
              voice->period -= voice->portaU;
              if (voice->period < 0) voice->period = 0;
              voice->flags |= UPDATE_PERIOD;
              break;
            case FX_PORTAMENTO_DOWN:
              voice->period += voice->portaD;
              if (voice->period > 9212) voice->period = 9212;
              voice->flags |= UPDATE_PERIOD;
              break;
            case FX_TONE_PORTAMENTO:
              if (voice->portaPeriod) voice->tonePortamento();
              break;
            case FX_VIBRATO:
              if (paramx) voice->vibratoSpeed = paramx;
              if (paramy) voice->vibratoDepth = paramy << 2;
              voice->vibrato();
              break;
            case FX_TONE_PORTA_VOLUME_SLIDE:
              slide = 1;
              if (voice->portaPeriod) voice->tonePortamento();
              break;
            case FX_VIBRATO_VOLUME_SLIDE:
              slide = 1;
              voice->vibrato();
              break;
            case FX_TREMOLO:
              voice->tremolo();
              break;
            case FX_VOLUME_SLIDE:
              slide = 1;
              break;
            case FX_EXTENDED_EFFECTS:

              switch (paramx) {
                case EX_RETRIG_NOTE:
                  if ((tick % paramy) == 0) {
                    voice->volEnvelope->reset();
                    voice->panEnvelope->reset();
                    voice->flags |= (UPDATE_VOLUME | UPDATE_PANNING | UPDATE_TRIGGER);
                  }
                  break;
                case EX_NOTE_CUT:
                  if (tick == paramy) {
                    voice->volume = 0;
                    voice->flags |= UPDATE_VOLUME;
                  }
                  break;
              }

              break;
            case FX_GLOBAL_VOLUME_SLIDE:
              paramx = voice->volSlideMaster >> 4;
              paramy = voice->volSlideMaster & 15;

              if (paramx) {
                master += paramx;
                if (master > 64) master = 64;
                voice->flags |= UPDATE_VOLUME;
              } else if (paramy) {
                master -= paramy;
                if (master < 0) master = 0;
                voice->flags |= UPDATE_VOLUME;
              }
              break;
            case FX_KEYOFF:
              if (tick == row->param) {
                voice->fadeEnabled = 1;
                voice->keyoff = 1;
              }
              break;
            case FX_PANNING_SLIDE:
              paramx = voice->panSlide >> 4;
              paramy = voice->panSlide & 15;

              if (paramx) {
                voice->panning += paramx;
                if (voice->panning > 255) voice->panning = 255;
                voice->flags |= UPDATE_PANNING;
              } else if (paramy) {
                voice->panning -= paramy;
                if (voice->panning < 0) voice->panning = 0;
                voice->flags |= UPDATE_PANNING;
              }
              break;
            case FX_MULTI_RETRIG_NOTE:
              com = tick;
              if (!row->volume) com++;
              if (com % voice->retrigy) break;

              if ((!row->volume || row->volume > 80) && voice->retrigx) retrig(voice);
              voice->flags |= UPDATE_TRIGGER;
              break;
            case FX_TREMOR:
              voice->tremor();
              break;
          }

          if (slide) {
            paramx = voice->volSlide >> 4;
            paramy = voice->volSlide & 15;
            slide = 0;

            if (paramx) {
              voice->volume += paramx;
              voice->flags |= UPDATE_VOLUME;
            } else if (paramy) {
              voice->volume -= paramy;
              voice->flags |= UPDATE_VOLUME;
            }
          }
          voice = voice->next;
        }
      }

      if (++tick >= (timer + patternDelay)) {
        patternDelay = tick = 0;

        if (nextPosition < 0) {
          nextPosition = position + channels;

          if (nextPosition >= pattern->size || complete) {
            nextOrder = order + 1;
            nextPosition = patternOffset;

            if (nextOrder >= length) {
              nextOrder = restart;
              mixer->complete = 1;
            }
          }
        }
      }
}

//override
void F2Player_fast(struct F2Player* self) {
      struct SBChannel *chan = NULL;
      int delta = 0; 
      int flags = 0; 
      struct F2Instrument *instr = NULL;
      int panning = 0;
      struct F2Voice *voice = voices[0];
      Number volume = NAN;

      while (voice) {
        chan  = voice->channel;
        flags = voice->flags;
        voice->flags = 0;

        if (flags & UPDATE_TRIGGER) {
          chan->index    = voice->sampleOffset;
          chan->pointer  = -1;
          chan->dir      =  0;
          chan->fraction =  0;
          chan->sample   = voice->sample;
          chan->length   = voice->sample->length;

          chan->enabled = chan->sample->data ? 1 : 0;
          voice->playing = voice->instrument;
          voice->sampleOffset = 0;
        }

        instr = voice->playing;
        delta = instr->vibratoSpeed ? voice->autoVibrato() : 0;

        volume = voice->volume + voice->volDelta;

        if (instr->volEnabled) {
          if (voice->volEnabled && !voice->volEnvelope->stopped)
            envelope(voice, voice->volEnvelope, instr->volData);

          volume = (volume * voice->volEnvelope->value) >> 6;
          flags |= UPDATE_VOLUME;

          if (voice->fadeEnabled) {
            voice->fadeVolume -= voice->fadeDelta;

            if (voice->fadeVolume < 0) {
              volume = 0;

              voice->fadeVolume  = 0;
              voice->fadeEnabled = 0;

              voice->volEnvelope->value   = 0;
              voice->volEnvelope->stopped = 1;
              voice->panEnvelope->stopped = 1;
            } else {
              volume = (volume * voice->fadeVolume) >> 16;
            }
          }
        } else if (voice->keyoff) {
          volume = 0;
          flags |= UPDATE_VOLUME;
        }

        panning = voice->panning;

        if (instr->panEnabled) {
          if (voice->panEnabled && !voice->panEnvelope->stopped)
            envelope(voice, voice->panEnvelope, instr->panData);

          panning = (voice->panEnvelope->value << 2);
          flags |= UPDATE_PANNING;

          if (panning < 0) panning = 0;
            else if (panning > 255) panning = 255;
        }

        if (flags & UPDATE_VOLUME) {
          if (volume < 0) volume = 0;
            else if (volume > 64) volume = 64;

          chan->volume = VOLUMES[int((volume * master) >> 6)];
          chan->lvol = chan->volume * chan->lpan;
          chan->rvol = chan->volume * chan->rpan;
        }

        if (flags & UPDATE_PANNING) {
          chan->panning = panning;
          chan->lpan = PANNING[int(256 - panning)];
          chan->rpan = PANNING[panning];

          chan->lvol = chan->volume * chan->lpan;
          chan->rvol = chan->volume * chan->rpan;
        }

        if (flags & UPDATE_PERIOD) {
          delta += voice->period + voice->arpDelta + voice->vibDelta;

          if (linear) {
            chan->speed  = int((548077568 * Math->pow(2, ((4608 - delta) / 768))) / 44100) / 65536;
          } else {
            chan->speed  = int((65536 * (14317456 / delta)) / 44100) / 65536;
          }

          chan->delta  = int(chan->speed);
          chan->speed -= chan->delta;
        }
        voice = voice->next;
      }
}

//override
void F2Player_accurate(struct F2Player* self) {
      struct SBChannel *chan = NULL;
      int delta = 0;
      int flags = 0; 
      struct F2Instrument *instr = NULL;
      Number lpan = NAN;
      Number lvol = NAN; 
      int panning = 0; 
      Number rpan = NAN; 
      Number rvol = NAN; 
      struct F2Voice *voice = voices[0];
      Number volume = NAN; 

      while (voice) {
        chan  = voice->channel;
        flags = voice->flags;
        voice->flags = 0;

        if (flags & UPDATE_TRIGGER) {
          if (chan->sample) {
            flags |= SHORT_RAMP;
            chan->mixCounter = 220;
            chan->oldSample  = null;
            chan->oldPointer = -1;

            if (chan->enabled) {
              chan->oldDir      = chan->dir;
              chan->oldFraction = chan->fraction;
              chan->oldSpeed    = chan->speed;
              chan->oldSample   = chan->sample;
              chan->oldPointer  = chan->pointer;
              chan->oldLength   = chan->length;

              chan->lmixRampD  = chan->lvol;
              chan->lmixDeltaD = chan->lvol / 220;
              chan->rmixRampD  = chan->rvol;
              chan->rmixDeltaD = chan->rvol / 220;
            }
          }

          chan->dir = 1;
          chan->fraction = 0;
          chan->sample  = voice->sample;
          chan->pointer = voice->sampleOffset;
          chan->length  = voice->sample->length;

          chan->enabled = chan->sample->data ? 1 : 0;
          voice->playing = voice->instrument;
          voice->sampleOffset = 0;
        }

        instr = voice->playing;
        delta = instr->vibratoSpeed ? voice->autoVibrato() : 0;

        volume = voice->volume + voice->volDelta;

        if (instr->volEnabled) {
          if (voice->volEnabled && !voice->volEnvelope->stopped)
            envelope(voice, voice->volEnvelope, instr->volData);

          volume = (volume * voice->volEnvelope->value) >> 6;
          flags |= UPDATE_VOLUME;

          if (voice->fadeEnabled) {
            voice->fadeVolume -= voice->fadeDelta;

            if (voice->fadeVolume < 0) {
              volume = 0;

              voice->fadeVolume  = 0;
              voice->fadeEnabled = 0;

              voice->volEnvelope->value   = 0;
              voice->volEnvelope->stopped = 1;
              voice->panEnvelope->stopped = 1;
            } else {
              volume = (volume * voice->fadeVolume) >> 16;
            }
          }
        } else if (voice->keyoff) {
          volume = 0;
          flags |= UPDATE_VOLUME;
        }

        panning = voice->panning;

        if (instr->panEnabled) {
          if (voice->panEnabled && !voice->panEnvelope->stopped)
            envelope(voice, voice->panEnvelope, instr->panData);

          panning = (voice->panEnvelope->value << 2);
          flags |= UPDATE_PANNING;

          if (panning < 0) panning = 0;
            else if (panning > 255) panning = 255;
        }

        if (!chan->enabled) {
          chan->volCounter = 0;
          chan->panCounter = 0;
          voice = voice->next;
          continue;
        }

        if (flags & UPDATE_VOLUME) {
          if (volume < 0) volume = 0;
            else if (volume > 64) volume = 64;

          volume = VOLUMES[int((volume * master) >> 6)];
          lvol = volume * PANNING[int(256 - panning)];
          rvol = volume * PANNING[panning];

          if (volume != chan->volume && !chan->mixCounter) {
            chan->volCounter = (flags & SHORT_RAMP) ? 220 : mixer->samplesTick;

            chan->lvolDelta = (lvol - chan->lvol) / chan->volCounter;
            chan->rvolDelta = (rvol - chan->rvol) / chan->volCounter;
          } else {
            chan->lvol = lvol;
            chan->rvol = rvol;
          }
          chan->volume = volume;
        }

        if (flags & UPDATE_PANNING) {
          lpan = PANNING[int(256 - panning)];
          rpan = PANNING[panning];

          if (panning != chan->panning && !chan->mixCounter && !chan->volCounter) {
            chan->panCounter = mixer->samplesTick;

            chan->lpanDelta = (lpan - chan->lpan) / chan->panCounter;
            chan->rpanDelta = (rpan - chan->rpan) / chan->panCounter;
          } else {
            chan->lpan = lpan;
            chan->rpan = rpan;
          }
          chan->panning = panning;
        }

        if (flags & UPDATE_PERIOD) {
          delta += voice->period + voice->arpDelta + voice->vibDelta;

          if (linear) {
            chan->speed = int((548077568 * Math->pow(2, ((4608 - delta) / 768))) / 44100) / 65536;
          } else {
            chan->speed  = int((65536 * (14317456 / delta)) / 44100) / 65536;
          }
        }

        if (chan->mixCounter) {
          chan->lmixRampU  = 0.0;
          chan->lmixDeltaU = chan->lvol / 220;
          chan->rmixRampU  = 0.0;
          chan->rmixDeltaU = chan->rvol / 220;
        }
        voice = voice->next;
      }
}

//override
void F2Player_initialize(struct F2Player* self) {
      int i = 0;
      struct F2Voice *voice = NULL;
      super->initialize();

      timer         = speed;
      order         =  0;
      position      =  0;
      nextOrder     = -1;
      nextPosition  = -1;
      patternDelay  =  0;
      patternOffset =  0;
      complete      =  0;
      master        = 64;

      voices = new Vector.<F2Voice>(channels, true);

      for (; i < channels; ++i) {
        voice = new F2Voice(i);

        voice->channel = mixer->channels[i];
        voice->playing = instruments[0];
        voice->sample  = voice->playing->samples[0];

        voices[i] = voice;
        if (i) voices[int(i - 1)].next = voice;
      }
}

//override
void F2Player_loader(struct F2Player* self, struct ByteArray *stream) {
      int header = 0;
      int i = 0;
      char id[24];
      int iheader = 0;
      struct F2Instrument *instr = NULL;
      int ipos = 0;
      int j = 0; 
      int len = 0; 
      struct F2Pattern *pattern = NULL;
      int pos = 0; 
      int reserved = 22;
      struct F2Row *row = NULL;
      int rows = 0; 
      struct F2Sample *sample = NULL;
      int value = 0;
      
      if (stream->length < 360) return;
      stream->position = 17;

      title = stream->readMultiByte(20, ENCODING);
      stream->position++;
      id = stream->readMultiByte(20, ENCODING);

      if (id == "FastTracker v2.00   " || id == "FastTracker v 2.00  ") {
        self->version = 1;
      } else if (id == "Sk@le Tracker") {
        reserved = 2;
        self->version = 2;
      } else if (id == "MadTracker 2.0") {
        self->version = 3;
      } else if (id == "MilkyTracker        ") {
        self->version = 4;
      } else if (id == "DigiBooster Pro 2.18") {
        self->version = 5;
      } else if (id->indexOf("OpenMPT") != -1) {
        self->version = 6;
      } else return;

      stream->readUnsignedShort();

      header   = stream->readUnsignedInt();
      length   = stream->readUnsignedShort();
      restart  = stream->readUnsignedShort();
      channels = stream->readUnsignedShort();

      value = rows = stream->readUnsignedShort();
      instruments = new Vector.<F2Instrument>(stream->readUnsignedShort() + 1, true);

      linear = stream->readUnsignedShort();
      speed  = stream->readUnsignedShort();
      tempo  = stream->readUnsignedShort();

      track = new Vector.<int>(length, true);

      for (i = 0; i < length; ++i) {
        j = stream->readUnsignedByte();
        if (j >= value) rows = j + 1;
        track[i] = j;
      }

      patterns = new Vector.<F2Pattern>(rows, true);

      if (rows != value) {
        pattern = new F2Pattern(64, channels);
        j = pattern->size;
        for (i = 0; i < j; ++i) pattern->rows[i] = new F2Row();
        patterns[--rows] = pattern;
      }

      stream->position = pos = header + 60;
      len = value;

      for (i = 0; i < len; ++i) {
        header = stream->readUnsignedInt();
        stream->position++;

        pattern = new F2Pattern(stream->readUnsignedShort(), channels);
        rows = pattern->size;

        value = stream->readUnsignedShort();
        stream->position = pos + header;
        ipos = stream->position + value;

        if (value) {
          for (j = 0; j < rows; ++j) {
            row = new F2Row();
            value = stream->readUnsignedByte();

            if (value & 128) {
              if (value &  1) row->note       = stream->readUnsignedByte();
              if (value &  2) row->instrument = stream->readUnsignedByte();
              if (value &  4) row->volume     = stream->readUnsignedByte();
              if (value &  8) row->effect     = stream->readUnsignedByte();
              if (value & 16) row->param      = stream->readUnsignedByte();
            } else {
              row->note       = value;
              row->instrument = stream->readUnsignedByte();
              row->volume     = stream->readUnsignedByte();
              row->effect     = stream->readUnsignedByte();
              row->param      = stream->readUnsignedByte();
            }

            if (row->note != KEYOFF_NOTE) if (row->note > 96) row->note = 0;
            pattern->rows[j] = row;
          }
        } else {
          for (j = 0; j < rows; ++j) pattern->rows[j] = new F2Row();
        }

        patterns[i] = pattern;
        pos = stream->position;
        if (pos != ipos) pos = stream->position = ipos;
      }

      ipos = stream->position;
      len = instruments->length;

      for (i = 1; i < len; ++i) {
        iheader = stream->readUnsignedInt();
        if ((stream->position + iheader) >= stream->length) break;

        instr = new F2Instrument();
        instr->name = stream->readMultiByte(22, ENCODING);
        stream->position++;

        value = stream->readUnsignedShort();
        if (value > 16) value = 16;
        header = stream->readUnsignedInt();
        if (reserved == 2 && header != 64) header = 64;

        if (value) {
          instr->samples = new Vector.<F2Sample>(value, true);

          for (j = 0; j < 96; ++j)
            instr->noteSamples[j] = stream->readUnsignedByte();
          for (j = 0; j < 12; ++j)
            instr->volData->points[j] = new F2Point(stream->readUnsignedShort(), stream->readUnsignedShort());
          for (j = 0; j < 12; ++j)
            instr->panData->points[j] = new F2Point(stream->readUnsignedShort(), stream->readUnsignedShort());

          instr->volData->total     = stream->readUnsignedByte();
          instr->panData->total     = stream->readUnsignedByte();
          instr->volData->sustain   = stream->readUnsignedByte();
          instr->volData->loopStart = stream->readUnsignedByte();
          instr->volData->loopEnd   = stream->readUnsignedByte();
          instr->panData->sustain   = stream->readUnsignedByte();
          instr->panData->loopStart = stream->readUnsignedByte();
          instr->panData->loopEnd   = stream->readUnsignedByte();
          instr->volData->flags     = stream->readUnsignedByte();
          instr->panData->flags     = stream->readUnsignedByte();

          if (instr->volData->flags & ENVELOPE_ON) instr->volEnabled = 1;
          if (instr->panData->flags & ENVELOPE_ON) instr->panEnabled = 1;

          instr->vibratoType  = stream->readUnsignedByte();
          instr->vibratoSweep = stream->readUnsignedByte();
          instr->vibratoDepth = stream->readUnsignedByte();
          instr->vibratoSpeed = stream->readUnsignedByte();
          instr->fadeout      = stream->readUnsignedShort() << 1;

          stream->position += reserved;
          pos = stream->position;
          instruments[i] = instr;

          for (j = 0; j < value; ++j) {
            sample = new F2Sample();
            sample->length    = stream->readUnsignedInt();
            sample->loopStart = stream->readUnsignedInt();
            sample->loopLen   = stream->readUnsignedInt();
            sample->volume    = stream->readUnsignedByte();
            sample->finetune  = stream->readByte();
            sample->loopMode  = stream->readUnsignedByte();
            sample->panning   = stream->readUnsignedByte();
            sample->relative  = stream->readByte();

            stream->position++;
            sample->name = stream->readMultiByte(22, ENCODING);
            instr->samples[j] = sample;

            stream->position = (pos += header);
          }

          for (j = 0; j < value; ++j) {
            sample = instr->samples[j];
            if (!sample->length) continue;
            pos = stream->position + sample->length;

            if (sample->loopMode & 16) {
              sample->bits       = 16;
              sample->loopMode  ^= 16;
              sample->length    >>= 1;
              sample->loopStart >>= 1;
              sample->loopLen   >>= 1;
            }

            if (!sample->loopLen) sample->loopMode = 0;
            sample->store(stream);
            if (sample->loopMode) sample->length = sample->loopStart + sample->loopLen;
            stream->position = pos;
          }
        } else {
          stream->position = ipos + iheader;
        }

        ipos = stream->position;
        if (ipos >= stream->length) break;
      }

      instr = new F2Instrument();
      instr->volData = new F2Data();
      instr->panData = new F2Data();
      instr->samples = new Vector.<F2Sample>(1, true);

      for (i = 0; i < 12; ++i) {
        instr->volData->points[i] = new F2Point();
        instr->panData->points[i] = new F2Point();
      }

      sample = new F2Sample();
      sample->length = 220;
      sample->data = new Vector.<Number>(220, true);

      for (i = 0; i < 220; ++i) sample->data[i] = 0.0;

      instr->samples[0] = sample;
      instruments[0] = instr;
}

void F2Player_envelope(struct F2Player* self, struct F2Voice *voice, struct F2Envelope *envelope, struct F2Data *data) {
      int pos = envelope->position;
      struct F2Point *curr = data->points[pos];
      struct F2Point *next = NULL;

      if (envelope->frame == curr->frame) {
        if ((data->flags & ENVELOPE_LOOP) && pos == data->loopEnd) {
          pos  = envelope->position = data->loopStart;
          curr = data->points[pos];
          envelope->frame = curr->frame;
        }

        if (pos == (data->total - 1)) {
          envelope->value = curr->value;
          envelope->stopped = 1;
          return;
        }

        if ((data->flags & ENVELOPE_SUSTAIN) && pos == data->sustain && !voice->fadeEnabled) {
          envelope->value = curr->value;
          return;
        }

        envelope->position++;
        next = data->points[envelope->position];

        envelope->delta = ((next->value - curr->value) << 8) / (next->frame - curr->frame);
        envelope->fraction = (curr->value << 8);
      } else {
        envelope->fraction += envelope->delta;
      }

      envelope->value = (envelope->fraction >> 8);
      envelope->frame++;
}

int F2Player_amiga(struct F2Player* self, int note, int finetune) {
      Number delta = 0.0;
      int period = PERIODS[++note];

      if (finetune < 0) {
        delta = (PERIODS[--note] - period) / 64;
      } else if (finetune > 0) {
        delta = (period - PERIODS[++note]) / 64;
      }

      return int(period - (delta * finetune));
}
    
void F2Player_retrig(struct F2Player* self, struct F2Voice *voice) {
      switch (voice->retrigx) {
        case 1:
          voice->volume--;
          break;
        case 2:
          voice->volume++;
          break;
        case 3:
          voice->volume -= 4;
          break;
        case 4:
          voice->volume -= 8;
          break;
        case 5:
          voice->volume -= 16;
          break;
        case 6:
          voice->volume = (voice->volume << 1) / 3;
          break;
        case 7:
          voice->volume >>= 1;
          break;
        case 8:
          voice->volume = voice->sample->volume;
          break;
        case 9:
          voice->volume++;
          break;
        case 10:
          voice->volume += 2;
          break;
        case 11:
          voice->volume += 4;
          break;
        case 12:
          voice->volume += 8;
          break;
        case 13:
          voice->volume += 16;
          break;
        case 14:
          voice->volume = (voice->volume * 3) >> 1;
          break;
        case 15:
          voice->volume <<= 1;
          break;
      }

      if (voice->volume < 0) voice->volume = 0;
        else if (voice->volume > 64) voice->volume = 64;

      voice->flags |= UPDATE_VOLUME;
}

