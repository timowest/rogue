/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdlib.h>
#include <math.h>
#include "tables.h"

namespace dsp {

// sintable

sintable sin_;

sintable::sintable() {
    for (int i = 0; i < 8001; i++) {
        values[i] = sin(2.0f * M_PI * float(i) / 8000.0f);
    }
}

float sintable::fast(float val) {
    // XXX doesn't check for boundaries
    return values[int(8000 * val)];
}

float sintable::linear(float val) {
    float pos = 8000.0f * val;
    float rem = pos - floor(pos);
    return rem * values[int(pos + 1)] + (1.0f - rem) * values[int(pos)];
}

// costable

costable cos_;

costable::costable() {
    for (int i = 0; i < 8001; i++) {
        values[i] = cos(2.0f * M_PI * float(i) / 8000.0f);
    }
}

float costable::fast(float val) {
    // XXX doesn't check for boundaries
    return values[int(8000 * val)];
}

float costable::linear(float val) {
    float pos = 8000.0f * val;
    float rem = pos - floor(pos);
    return rem * values[int(pos + 1)] + (1.0f - rem) * values[int(pos)];
}


}
