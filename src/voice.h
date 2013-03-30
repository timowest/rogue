/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_VOICE_H
#define ROGUE_VOICE_H

#include "common.h"
#include "rogue.peg"

#include <daps/synth.hpp>

namespace rogue {

class rogueVoice : public daps::Voice {
    private:
      float env, volume;
      short sustain;

    protected:
      unsigned char m_key;

    public:
      rogueVoice(double);
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
