/*
 * rogue - multimode synth
 *
 * contains main class for synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_SYNTH_H
#define ROGUE_SYNTH_H
//See associated .cpp file for copyright and other info

#include "common.h"
#include "config.h"
#include "voice.h"
#include "rogue.gen"

#include <lvtk/synth.hpp>
#include <stdio.h>

namespace rogue {

class rogueSynth : public lvtk::Synth<rogueVoice, rogueSynth> {

  public:
    rogueSynth(double);
    unsigned find_free_voice(unsigned char, unsigned char);
    void handle_midi(uint32_t, unsigned char*);
    void pre_process(uint32_t from, uint32_t to);
    void post_process(uint32_t from, uint32_t to);
    void set_volume(float);
    void update();

    template <typename T>
    T v(uint32_t port) {
        float* pv = p(port);
        return (T)pv;
    }

    float v(uint32_t port) {
        return *p(port);
    }

  private:
    float sample_rate;
    dsp::DCBlocker ldcBlocker, rdcBlocker;
    rogueVoice *voices[NVOICES];
    bool sustain;
    SynthData data;
};

}

#endif
