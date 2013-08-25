/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdio.h>
#include "envelope.h"
#include "types.h"

namespace dsp {

// AHDSR

void AHDSR::on() {
    state_ = preDelaySamples > 0.0f ? PRE : A;
    counter = preDelaySamples;
    offset = offset + scale * envCurve(last);
    scale = attackTarget - offset;
    last = 0.0f;
}

void AHDSR::off() {
    state_ = R;
    offset = offset + scale * envCurve(last);
    scale = -offset;
    last = 0.0f;
}

void AHDSR::setAHDSR(float _a, float _h, float _d, float _s, float _r) {
    attackRate = 1.0f / _a;
    holdSamples = _h;
    decayRate = 1.0f / _d;
    sustain = _s;
    releaseRate = 1.0f / _r;
}

float AHDSR::innerTick() {
    if (state_ == PRE) {      // pre-delay
        if (--counter == 0) {
            state_ = A;
        }
    } else if (state_ == A) { // attack
        last += attackRate;
        if (last >= 1.0f) {
            if (holdSamples > 0.0f) {
                state_ = H;
                counter = holdSamples;
                last = 1.0f;
            } else {
                state_ = D;
                scale = sustain - attackTarget;
                offset = attackTarget;
                last = 0.0f;
            }
        }
    } else if (state_ == H) { // hold
        if (--counter == 0) {
            state_ = D;
            scale = sustain - attackTarget;
            offset = attackTarget;
            last = 0.0f;
        }
    } else if (state_ == D) { // decay
        last += decayRate;
        if (last >= 1.0f) {
            state_ = S;
            scale = -sustain;
            offset = sustain;
            last = 0.0f;
        }
    } else if (state_ == R) { // release
        last += releaseRate;
        if (last >= 1.0f) {
            if (retrigger) {
                on();
            } else {
                state_ = IDLE;
                scale = offset = 0.0f;
                last = 0.0f;
            }
        }
    }
    return last;
}

float AHDSR::envCurve(float x) {
    return x / (x + a*(x-1.0f));
}

float AHDSR::tick() {
    innerTick();
    return offset + scale * envCurve(last);
}

float AHDSR::tick(int samples) {
    // TODO optimize
    for (uint i = 0; i < samples; i++) {
        innerTick();
    }
    return offset + scale * envCurve(last);
}

}
