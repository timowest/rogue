/*
 * rogue - multimode synth
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
    float param1, param2;
    float level_a, level_b;
    float level, prev_level;

    // modulation
    float vel_to_vol;
};

struct FilterData {
    bool on;
    int type, source;
    float freq, q, distortion;
    float level, prev_level;
    float pan;

    // modulation
    float key_to_f;
    float vel_to_f;
};

struct LFOData {
    bool on;
    int type, reset_type;
    bool inv;
    float freq;
    float symmetry, attack, decay;
    float humanize;
};

struct EnvData {
    bool on;
    float pre_delay;
    float attack, hold, decay, sustain, release;
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

    float bus_a_level, bus_a_pan;
    float bus_b_level, bus_b_pan;
    float volume;
    float glide_time, bend_range;
};

}

#endif

