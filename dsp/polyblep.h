/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_POLYBLEP_H
#define DSP_POLYBLEP_H

#include <math.h>
#include "types.h"

namespace dsp {

static float polyblep(float t) {
    if (t > 0.0f) {
        return t - (t*t)/2.0f - 0.5f;
    } else {
        return (t*t)/2.0f + t + 0.5f;
    }
}

static float saw_polyblep(float p, float inc) {
    float mod = 0.0f;
    if (p < inc) { // start
        mod = polyblep(p / inc);
    } else if (p > (1.0f - inc)) { // end
        mod = polyblep((p - 1.0f) / inc);
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

// sync variants

static float saw_sync(float p, float p_, float inc, float sync) {
    float mod = 0.0f;
    if (sync >= 0.0f) { // sync start
        mod = (p_ + inc - p) * polyblep(sync);
    } else if (sync > -1.0f) { // sync end
        mod = (p + (1.0 + sync) * inc) * polyblep(sync);
    } else if (p < inc) { // start
        mod = polyblep(p / inc);
    } else if (p > (1.0f - inc)) { // end
        mod = polyblep((p - 1.0f) / inc);
    }
    return mod;
}

static float pulse_sync(float p, float p_, float width, float inc, float sync) {
    float mod = 0.0f;
    if (p < width) {
        if (p < inc) { // start
            if (sync < 0.0f || p_ >= (width - inc)) {
                mod = polyblep(p / inc);
            }
        } else if (p > (width - inc)) {
            mod = -polyblep( (p - width) / inc);
        }
    } else {
        if (p > (1.0f - inc)) { // end
            mod = polyblep( (p - 1.0f) / inc);
        } else if (p < (width + inc)) {
            mod = -polyblep( (p - width) / inc);
        } else if (sync > -1.0f) {
            mod = polyblep(sync);
        }
    }
    return mod;
}

}

#endif
