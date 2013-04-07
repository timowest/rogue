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
    float values[8001];
    sintable();
    float fast(float in);
    float linear(float in);
};

extern sintable sin_;

}

#endif
