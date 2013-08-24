/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_POLYBLEP_H
#define DSP_POLYBLEP_H

namespace dsp {

static float polyblep(float t) {
    if (t > 0.0f) {
        return t - (t*t)/2.0f - 0.5f;
    } else {
        return (t*t)/2.0f + t + 0.5f;
    }
}

static float saw_polyblep(float p, float max, float inc) {
    float mod = 0.0f;
    if (p < inc) { // start
        mod = polyblep(p / inc);
    } else if (p > (max - inc)) { // end
        mod = polyblep((p - max) / inc);
    }
    return mod;
}

static float pulse_polyblep(float p, float width, float inc) {
    float mod = 0.0f;
    if (p < width) {
        if (p < inc) { // start
            mod = polyblep(p / inc);
        } else if (p > (width - inc)) {
            mod = -polyblep( (p - width) / inc);
        }
    } else {
        if (p > (1.0f - inc)) { // end
            mod = polyblep( (p - 1.0f) / inc);
        } else if (p < (width + inc)) {
            mod = -polyblep( (p - width) / inc);
        }
    }
    return mod;
}

}

#endif
