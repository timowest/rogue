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

// effects
#include "basics.h"
#include "Chorus.h"
#include "Phaser.h"
#include "Scape.h"
#include "Reverb.h"
#include "Descriptor.h"

namespace rogue {

class rogueSynth : public lvtk::Synth<rogueVoice, rogueSynth> {

    enum {POLY, MONO, LEGATO};

  public:
    rogueSynth(double);
    unsigned find_free_voice(unsigned char, unsigned char);
    void handle_midi(uint32_t, unsigned char*);
    void pre_process(uint32_t from, uint32_t to);
    void post_process(uint32_t from, uint32_t to);
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

    StereoChorusII2x2 chorus;
    float* chorus_ports[10];
    StereoPhaserII2x2 phaser;
    float* phaser_ports[8];
    Scape delay;
    float* delay_ports[9];
    Plate2x2 reverb;
    float* reverb_ports[8];
    bool effects_activated = false;
};

}

#endif
