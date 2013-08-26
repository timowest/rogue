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

float AHDSR::innerTick(int samples) {
    while (true) {
        if (state_ == PRE) {      // pre-delay
            counter -= samples;
            if (counter <= 0) {
                state_ = A;
                if (counter < 0) {
                    samples = -counter;
                    counter = 0;
                    continue;
                }
            }
        } else if (state_ == A) { // attack
            last += samples * attackRate;
            if (last >= 1.0f) {
                samples = (last - 1.0) / attackRate;
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
                if (samples > 0) {
                    continue;
                }
            }
        } else if (state_ == H) { // hold
            counter -= samples;
            if (counter <= 0) {
                state_ = D;
                scale = sustain - attackTarget;
                offset = attackTarget;
                last = 0.0f;
                if (counter < 0) {
                    samples = -counter;
                    counter = 0;
                    continue;
                }
            }
        } else if (state_ == D) { // decay
            last += samples * decayRate;
            if (last >= 1.0f) {
                samples = (last - 1.0) / decayRate;
                state_ = S;
                scale = -sustain;
                offset = sustain;
                last = 0.0f;
                if (samples > 0) {
                    continue;
                }
            }
        } else if (state_ == R) { // release
            last += samples * releaseRate;
            if (last >= 1.0f) {
                samples = (last - 1.0) / releaseRate;
                if (retrigger) {
                    last = 1.0;
                    on();
                } else {
                    state_ = IDLE;
                    scale = offset = 0.0f;
                    last = 0.0f;
                    break;
                }
                if (samples > 0) {
                    continue;
                }
            }
        }
        break;
    }

    return last;
}

float AHDSR::envCurve(float x) {
    return x / (x + a*(x-1.0f));
}

float AHDSR::tick() {
    innerTick(1);
    return offset + scale * envCurve(last);
}

float AHDSR::tick(int samples) {
    innerTick(samples);
    return offset + scale * envCurve(last);
}

}
