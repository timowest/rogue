/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdlib.h>
#include <math.h>
#include "tables.h"

namespace dsp {

sintable sin_;

sintable::sintable() {
    for (int i = 0; i < 8001; i++) {
        values[i] = sin(2.0 * M_PI * float(i) / 8000);
    }
}

float sintable::fast(float val) {
    // XXX doesn't check for boundaries
    return values[int(8000 * val)];
}

float sintable::linear(float val) {
    float pos = 8000.0 * val;
    float rem = pos - floor(pos);
    return rem * values[int(pos + 1)] + (1.0 - rem) * values[int(pos)];
}

}
