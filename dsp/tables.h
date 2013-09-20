/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_TABLES_H
#define DSP_TABLES_H

namespace dsp {

/**
 * sin table (range 0 - 1)
 */
struct sintable {
    float values[8010];
    sintable();
    float fast(float in);
    float linear(float in);
};

extern sintable sin_;

/**
 * cos table (range 0 - 1)
 */
struct costable {
    float values[8010];
    costable();
    float fast(float in);
    float linear(float in);
};

extern costable cos_;

/*
 * tanh table (range -5 to 5)
 *
 */
struct tanhtable {
    float values[16010];
    tanhtable();
    float fast(float in);
    float linear(float in);
};

extern tanhtable tanh_;

}

#endif
