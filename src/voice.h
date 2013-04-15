/*
 * rogue - multimode synth
 *
 * contains dsp element wrappers and voice class
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_VOICE_H
#define ROGUE_VOICE_H

#include <lvtk/synth.hpp>

#include "common.h"
#include "config.h"
#include "rogue.gen"
#include "dsp.h"

namespace rogue {

struct Osc {
    dsp::PhaseShaping osc;
    float buffer[BUFFER_SIZE];
    float prev_level;
    void reset() { osc.reset(); }

    void setSamplerate(float r) {
        osc.setSamplerate(r);
    }
};

struct Filter {
    dsp::MoogFilter moog;
    dsp::StateVariableFilter svf;
    float buffer[BUFFER_SIZE];
    float prev_level;
    float key_vel_to_f;

    void setSamplerate(float r) {
        moog.setSamplerate(r);
        svf.setSamplerate(r);
    }
};

struct LFO {
    dsp::LFO lfo;
    float current, last;
    void on() {}
    void off() {}
};

struct Env {
    dsp::AHDSR env;
    float current, last;
    void on() { env.on(); }
    void off() { env.off(); }
};

class rogueVoice : public lvtk::Voice {
    private:
      float volume = 1.0f;
      SynthData* data;
      Osc oscs[NOSC];
      Filter filters[NDCF];
      LFO lfos[NLFO];
      Env envs[NENV];

      float* left;
      float* right;
      float* buffers[4];
      float bus_a[BUFFER_SIZE], bus_b[BUFFER_SIZE];
      float mod[M_SIZE];
      bool in_sustain = false;

    protected:
      float sample_rate;
      unsigned char m_key, m_velocity;

      float pitch_modulate(int target);
      float modulate(int target);

      // configure
      void configLFO(int i);
      void configEnv(int i);
      void configOsc(int i);
      void configFilter(int i);

      // run
      void runLFO(int i, uint32_t from, uint32_t to);
      void runEnv(int i, uint32_t from, uint32_t to);
      void runOsc(int i, uint32_t from, uint32_t to);
      void runFilter(int i, uint32_t from, uint32_t to);

      void render(uint32_t, uint32_t, uint32_t off);

    public:
      rogueVoice(double, SynthData*);
      void set_volume(float v) { volume = v; }
      void on(unsigned char key, unsigned char velocity);
      void off(unsigned char velocity);
      void reset(void);
      bool is_sustained(void) { return in_sustain; }
      unsigned char get_key(void) const { return m_key; }

      // generates the sound for this voice
      void render(uint32_t, uint32_t);
};

}

#endif
