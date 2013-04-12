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
    bool inv; // output inversion
    bool free; // phase reset
    bool tracking; // pitch tracking
    float ratio;
    float coarse; //  tune
    float fine; // tune
    float param1;
    float param2;
    float level_a;
    float level_b;
    float level;

    // modulation
    float vel_to_vol;
    int   pitch_m_src, mod_m_src, amp_m_src;
    float pitch_m_amt, mod_m_amt, amp_m_amt;
};

struct FilterData {
    bool on;
    int type;
    int source;
    float freq;
    float q;
    float distortion;
    float level;
    float pan;

    // modulation
    float key_to_f;
    float vel_to_f;
    int freq_m_src, q_m_src, pan_m_src, amp_m_src;
    float freq_m_amt, q_m_amt, pan_m_amt, amp_m_amt;
};

struct LFOData {
    bool on;
    int type;
    int reset_type;
    float freq;
    float symmetry;
    float attack;
    float decay;
    float humanize;

    // modulation
    float key_to_f;
    int speed_m_src, amp_m_src;
    float speed_m_amt, amp_m_amt;
};

struct EnvData {
    bool on;
    float pre_delay;
    float attack;
    float hold;
    float decay;
    float sustain;
    float release;
    bool  retrigger;
    float vel_to_vol;

    // modulation
    float key_to_speed;
    float vel_to_speed;
    int amp_m_src;
    float amp_m_amt;
};

struct DelaylineData {
    // TODO
};

struct SynthData {
    OscData oscs[NOSC];
    FilterData filters[NDCF];
    LFOData lfos[NLFO];
    EnvData envs[NENV];

    float bus_a_level, bus_a_pan;
    float bus_b_level, bus_b_pan;
    float volume;
    float glide_time, bend_range;
};

}

#endif

