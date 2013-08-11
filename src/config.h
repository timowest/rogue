/*
 * rogue - multimode synth
 *
 * contains shared configuration for synth elements
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_CONFIG_H
#define ROGUE_CONFIG_H

#include "common.h"

namespace rogue {

struct OscData {
    bool on;
    int type;
    bool inv, free, tracking;
    float ratio, coarse, fine;
    float start, width;
    float level_a, level_b;
    float level;

    // modulation
    int input, out_mod;
    float pm;
    bool sync;
};

struct FilterData {
    bool on;
    int type, source;
    float freq, q, distortion;
    float level, pan;

    // modulation
    float key_to_f;
    float vel_to_f;
};

struct LFOData {
    bool on;
    int type, reset_type;
    bool inv;
    float freq;
    float start, width;
    float humanize;
};

struct EnvData {
    bool on;
    float pre_delay;
    float attack, hold, decay, sustain, release;
    float curve;
    bool retrigger;
};

struct DelaylineData {
    // TODO
};

struct ModulationData {
    unsigned int src, target;
    float amount;
};

struct SynthData {
    OscData oscs[NOSC];
    FilterData filters[NDCF];
    LFOData lfos[NLFO];
    EnvData envs[NENV];
    ModulationData mods[NMOD];

    float pitch_bend;
    int playmode;
    float bus_a_level, bus_a_pan;
    float bus_b_level, bus_b_pan;
    float volume;
    float glide_time, bend_range;
};

}

#endif

