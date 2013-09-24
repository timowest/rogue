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

    enum {POLY, MONO, LEGATO};

    private:
      SynthData* data;
      Osc oscs[NOSC];
      Filter filters[NDCF];
      LFO lfos[NLFO];
      Env envs[NENV];

      float glide_target;
      float* buffers[4];
      float bus_a[BUFFER_SIZE], bus_b[BUFFER_SIZE];
      float mod[M_SIZE];
      bool in_sustain = false;

      float* left;
      float* right;

    protected:
      float sample_rate;
      float half_sample_rate;
      unsigned char m_key, m_velocity;
      float key, velocity, glide_step;

      template<class Function>
      float modulate(float init, int target, Function fn);

      // configure
      void configLFO(uint i);
      void configEnv(uint i);
      void configOsc(uint i);
      void configFilter(uint i);

      // run
      void runLFO(uint i, uint from, uint to);
      void runEnv(uint i, uint from, uint to);
      void runOsc(uint i, uint from, uint to);
      void runFilter(uint i, uint from, uint to);

      void render(uint, uint, uint off);

    public:
      rogueVoice(double, SynthData*, float*, float*);
      void on(unsigned char key, unsigned char velocity);
      void off(unsigned char velocity);
      void reset();
      bool is_sustained() { return in_sustain; }
      unsigned char get_key() const { return m_key; }

      // generates the sound for this voice
      void render(uint, uint);
};

}

#endif
