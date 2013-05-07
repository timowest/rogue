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
    type = 0;
    phase = 0.0;
    prev_phase = 0.0;
    value = 0.0;
}

void LFO::reset() {
    phase = 0.0;
    prev_phase = 0.0;
    value = 0.0;
}

float LFO::getValue(float p) {
    switch (type) {
    case SIN:
        return sin_.fast(p);
    case TRI:
        return 2.0f * (p < 0.5f ? 2.0f * p : 2.0f - 2.0f * p) - 1.0f;
    case SAW:
        return 2.0f * p - 1.0f;
    case SQUARE:
        return p < width ? -1.0 : 1.0;
    case SH:
        if (prev_phase > p) { // update value once per cycle
            value = (2.0f * rand() / float(RAND_MAX + 1.0f) - 1.0f);
        }
        prev_phase = p;
        return value;
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
