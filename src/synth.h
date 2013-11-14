/*
 * rogue - multimode synth
 *
 * contains main class for synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_SYNTH_H
#define ROGUE_SYNTH_H

#include <samplerate.h>

#include "common.h"
#include "config.h"
#include "voice.h"
#include "rogue.gen"
#include "effects.h"

#include <lvtk/synth.hpp>
#include <stdio.h>

namespace rogue {

class rogueSynth : public lvtk::Synth<rogueVoice, rogueSynth> {

    enum {POLY, MONO, LEGATO};

  public:
    rogueSynth(double);
    ~rogueSynth();

    unsigned find_free_voice(unsigned char, unsigned char);
    void handle_midi(uint, unsigned char*);
    void pre_process(uint from, uint to);
    void post_process(uint from, uint to);
    void update();

  private:
    float sample_rate;
    dsp::DCBlocker ldcBlocker, rdcBlocker;
    rogueVoice *voices[NVOICES];
    bool sustain;
    SynthData data;

    SRC_STATE* converter_l;
    SRC_STATE* converter_r;
    SRC_DATA converter_data;
    float left[8192];
    float right[8192];

    dsp::ChorusEffect chorus_fx;
    dsp::PhaserEffect phaser_fx;
    dsp::DelayEffect  delay_fx;
    dsp::ReverbEffect reverb_fx;
};

}

#endif
