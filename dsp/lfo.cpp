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
    attack = decay = 0.0;
    phase = 0.0;
}

void LFO::reset() {
    phase = 0.0;
}

void LFO::setEnv(float a, float d) {
    attack = a;
    decay = d;
}

void LFO::on() {
    // TODO
}

void LFO::off() {
    // TODO
}

float LFO::getValue(float p) {
    switch (type) {
    case SIN:
        return sin_.fast(p);
    case TRI:
        return 2.0 * (p < 0.5 ? 2.0 * p : 2.0 - 2.0 * p) - 1.0;
    case SAW_UP:
        return 2.0 * p - 1.0;
    case SAW_DOWN:
        return 1.0 - 2.0 * p;
    case SQUARE:
        return p < symmetry ? -1.0 : 1.0;
    case SH:
        return (2.0 * rand() / (RAND_MAX + 1.0) - 1.0);
    default:
        return 0.0;
    }
}

float LFO::tick() {
    float inc = freq / sample_rate;
    phase = fmod(phase + inc, 1.0);
    return getValue(phase);
}

float LFO::tick(int samples) {
    float inc = freq / sample_rate;
    phase = fmod(phase + float(samples) * inc, 1.0);
    return getValue(phase);
}

}
