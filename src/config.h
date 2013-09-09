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
    bool on = false;
    uint type = 0;
    bool inv, free, tracking;
    float ratio, coarse, fine;
    float start, width;
    float level_a, level_b;
    float level;

    // modulation
    uint input = 0, out_mod = 0;
    float pm;
    bool sync;
};

struct FilterData {
    bool on = false;
    uint type = 0, source = 0;
    float freq, q, distortion;
    float level, pan;

    // modulation
    float key_to_f;
    float vel_to_f;
};

struct LFOData {
    bool on = false;
    uint type = 0, reset_type = 0;
    bool inv;
    float freq;
    float start, width;
    float humanize;

    float phase = 0.0f;
};

struct EnvData {
    bool on = false;
    float pre_delay;
    float attack, hold, decay, sustain, release;
    float curve;
    bool retrigger;
};

struct DelaylineData {
    // TODO
};

struct ModulationData {
    uint src = 0, target = 0;
    float amount = 0.0;
};

struct SynthData {
    OscData oscs[NOSC];
    FilterData filters[NDCF];
    LFOData lfos[NLFO];
    EnvData envs[NENV];
    ModulationData mods[NMOD];
    uint mod_count;

    float pitch_bend;
    uint playmode;
    float bus_a_level, bus_a_pan;
    float bus_b_level, bus_b_pan;
    float volume;
    float glide_time, bend_range;
};

}

#endif

