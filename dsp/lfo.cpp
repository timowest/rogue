/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdlib.h>
#include <math.h>
#include "lfo.h"
#include "tables.h"

namespace dsp {

// LFO

void LFO::clear() {
    //type = 0;
    phase = start;
    prev_phase = start;
}

void LFO::reset() {
    phase = start;
    prev_phase = start;
}

float LFO::getValue(float p) {
    float val;
    switch (type) {
    case SIN:
        return sin_.fast(p);
    case TRI:
        if (p < width) {
            return 2.0 * (p/width) - 1.0;
        } else {
            return 2.0 * (1.0 - (p-width) / (1.0-width)) - 1.0;
        }
    case SAW:
        return 2.0f * p - 1.0f;
    case PULSE:
        return p < width ? -1.0 : 1.0;
    case SH:
        if (prev_phase > p) { // update value once per cycle
            val = (2.0f * rand() / float(RAND_MAX + 1.0f) - 1.0f);
        }
        prev_phase = p;
        return val;
    case NOISE:
        return (2.0f * rand() / float(RAND_MAX + 1.0f) - 1.0f);
    default:
        return 0.0f;
    }
}

float LFO::tick() {
    float inc = freq / sample_rate;
    phase = fmod(phase + inc, 1.0f);
    return getValue(phase);
}

float LFO::tick(int samples) {
    float inc = freq / sample_rate;
    phase = fmod(phase + float(samples) * inc, 1.0f);
    return getValue(phase);
}

}
