/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_CONFIG_H
#define ROGUE_CONFIG_H

namespace rogue {

struct OscillatorData {
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
    float volume;

    // modulation
    float vel_to_vol;
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
};

struct EnvelopeData {
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
};

struct DelaylineData {
    // TODO
};

struct SynthData {
    OscillatorData oscs[4];
    FilterData filters[2];
    LFOData lfos[3];
    EnvelopeData envs[5];

    float bus_a_level, bus_a_pan;
    float bus_b_level, bus_b_pan;
    float volume;
    float glide_time, bend_range;
};

}

#endif

