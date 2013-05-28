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
#include "wrappers.h"

namespace rogue {

class rogueVoice : public lvtk::Voice {
    private:
      float volume = 1.0f;
      SynthData* data;
      Osc oscs[NOSC];
      Filter filters[NDCF];
      LFO lfos[NLFO];
      Env envs[NENV];

      float* buffers[4];
      float bus_a[BUFFER_SIZE], bus_b[BUFFER_SIZE];
      float mod[M_SIZE];
      bool in_sustain = false;

    protected:
      float sample_rate;
      unsigned char m_key, m_velocity;

      template<class Function>
      float modulate(float init, int target, Function fn);

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
