/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_SYNTH_H
#define ROGUE_SYNTH_H
//See associated .cpp file for copyright and other info

#include "common.h"
#include "config.h"
#include "voice.h"
#include "rogue.peg"

#include <daps/synth.hpp>
#include <stdio.h>

namespace rogue {

class rogueSynth : public daps::Synth<rogueVoice, rogueSynth> {

  public:
    rogueSynth(double);
    unsigned find_free_voice(unsigned char, unsigned char);
    void handle_midi(uint32_t, unsigned char*);
    void pre_process(uint32_t from, uint32_t to);
    void set_volume(float);
    void update();

  private:
    float sample_rate;
    rogueVoice *voices[NVOICES];
    bool sustain;
    SynthData data;

};

}

#endif
