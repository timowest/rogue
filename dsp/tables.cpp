/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdlib.h>
#include <math.h>
#include "tables.h"
#include "types.h"

namespace dsp {

// sintable

sintable sin_;

sintable::sintable() {
    for (uint i = 0; i < 8010; i++) {
        values[i] = sin(2.0f * M_PI * float(i) / 8000.0f);
    }
}

float sintable::fast(float val) {
    // XXX doesn't check for boundaries
    return values[int(8000 * val) % 8000];
}

float sintable::linear(float val) {
    float pos = 8000.0f * val;
    float rem = pos - floor(pos);
    return rem * values[int(pos + 1)] + (1.0f - rem) * values[int(pos)];
}

// costable

costable cos_;

costable::costable() {
    for (uint i = 0; i < 8010; i++) {
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

// tanhtable

tanhtable tanh_;

tanhtable::tanhtable() {
    for (uint i = 0; i < 16010; i++) {
        values[i] = tanh(float(int(i) - 8000) / 1600.0f);
    }
}

float tanhtable::fast(float val) {
    if (val > 5.0f) {
        return 1.0f;
    } else if (val < -5.0f) {
        return -1.0f;
    }
    return values[int(1600.0f * (val + 5.0f))];
}

float tanhtable::linear(float val) {
    if (val > 5.0f) {
        return 1.0f;
    } else if (val < -5.0f) {
        return -1.0f;
    }
    float pos = 1600.0f * (val + 5.0f);
    float rem = pos - floor(pos);
    return rem * values[int(pos + 1)] + (1.0f - rem) * values[int(pos)];
}


}
