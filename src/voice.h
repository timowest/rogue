/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_VOICE_H
#define ROGUE_VOICE_H

#include "common.h"
#include "config.h"
#include "rogue.peg"
#include "dsp.h"

#include <daps/synth.hpp>

namespace rogue {

struct rogueOsc {
    dsp::PhaseShaping osc;
    float buffer[BUFFER_SIZE];
};

struct rogueFilter {
    dsp::MoogFilter moog;
    dsp::StateVariableFilter svf;
    float buffer[BUFFER_SIZE];
};

struct rogueLfo {
    // TODO
    float current, last;
};

struct rogueEnv {
    dsp::ADSR adsr;
    float current, last;
};

class rogueVoice : public daps::Voice {
    private:
      float env, volume;
      short sustain;
      SynthData* data;
      rogueOsc oscs[4];
      rogueFilter filters[2];
      rogueLfo lfos[3];
      rogueEnv envs[5];

      float bus_a[BUFFER_SIZE], bus_b[BUFFER_SIZE];

    protected:
      float sample_rate;
      unsigned char m_key;

    public:
      rogueVoice(double, SynthData*);
      void set_sustain(unsigned short v) { sustain = v; }
      void set_volume(float v) { volume = v; }
      void on(unsigned char key, unsigned char velocity);
      void release(unsigned char velocity);
      void reset(void);
      bool is_sustained(void) { return (m_key == SUSTAIN); }
      unsigned char get_key(void) const { return m_key; }

      // generates the sound for this voice
      void render(uint32_t, uint32_t);
};

}

#endif
