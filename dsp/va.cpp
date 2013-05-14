/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "va.h"
#include "tables.h"

namespace dsp {

// VA

void VA::clear() {
    phase = 0.0f;
    freq = 440.0f;
    type = 0;

    width = 0.5;
}

void VA::reset() {
    phase = 0.0;
}

float mod_saw(float x, float P) {
    float m = M_PI - 2.0 * M_PI * P;
    if (x < P) {
        return m * (x / P);
    } else {
        return m * (1.0f - x) / (1.0f - P);
    }
}

void VA::process(float* output, int samples) {
    float inc = freq / sample_rate;

    // TRI, SAW, SAW_TRI, SQUARE, PULSE

    if (type == TRI) {
        // TODO
    } else if (type == SAW) {
        // TODO optimize
        float P = 0.9924f - 0.00002151 * freq;
        for (int i = 0; i < samples; i++) {
            output[i] = sin(2.0 * M_PI * phase + mod_saw(phase, P) - 0.5 * M_PI);
            phase = fmod(phase + inc, 1.0f);
        }
    } else if (type == SAW_TRI) {
        // TODO
    } else if (type == SQUARE) {
        // TODO
    } else {
        // TODO
    }
}

}
