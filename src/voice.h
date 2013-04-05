/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_VOICE_H
#define ROGUE_VOICE_H

#include "common.h"
#include "config.h"
#include "rogue.gen"
#include "dsp.h"

#include <lvtk/synth.hpp>

namespace rogue {

struct rogueOsc {
    dsp::PhaseShaping osc;
    float buffer[BUFFER_SIZE];
    void reset() { osc.reset(); }

    void setSamplerate(float r) {
        osc.setSamplerate(r);
    }
};

struct rogueFilter {
    dsp::MoogFilter moog;
    dsp::StateVariableFilter svf;
    float buffer[BUFFER_SIZE];

    void setSamplerate(float r) {
        moog.setSamplerate(r);
        svf.setSamplerate(r);
    }
};

struct rogueLFO {
    dsp::LFO lfo;
    float current, last;
    void on() { lfo.on(); }
    void off() { lfo.off(); }
};

struct rogueEnv {
    dsp::ADSR adsr;
    float current, last;
    void on() { adsr.on(); }
    void off() { adsr.off(); }
};

class rogueVoice : public lvtk::Voice {
    private:
      float env = 0.0, volume = 1.0f;
      short sustain = 0;
      SynthData* data;
      rogueOsc oscs[NOSC];
      rogueFilter filters[NDCF];
      rogueLFO lfos[NLFO];
      rogueEnv envs[NENV];

      float bus_a[BUFFER_SIZE], bus_b[BUFFER_SIZE];
      float mod[M_SIZE];

    protected:
      float sample_rate;
      unsigned char m_key, m_velocity;

    public:
      rogueVoice(double, SynthData*);
      void set_sustain(unsigned short v) { sustain = v; }
      void set_volume(float v) { volume = v; }
      void on(unsigned char key, unsigned char velocity);
      void off(unsigned char velocity);
      void reset(void);
      bool is_sustained(void) { return (m_key == SUSTAIN); }
      unsigned char get_key(void) const { return m_key; }

      void runLFO(int i, uint32_t from, uint32_t to);
      void runEnv(int i, uint32_t from, uint32_t to);
      void runOsc(int i, uint32_t from, uint32_t to);
      void runFilter(int i, uint32_t from, uint32_t to);

      // generates the sound for this voice
      void render(uint32_t, uint32_t);
};

}

#endif
