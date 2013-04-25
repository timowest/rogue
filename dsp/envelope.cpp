/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdio.h>
#include "envelope.h"

namespace dsp {

// ADSR

void ADSR::on() {
    // don't reset when retriggered during release
    if (state_ == IDLE) {
        last = 0.0;
    }
    state_ = A;
}

void ADSR::off() {
    state_ = R;
    releaseRate = last / releaseSamples;
}

void ADSR::setADSR(float _a, float _d, float _s, float _r) {
    attackRate = attackTarget / _a;
    decayRate = (attackTarget - _s) / _d;
    sustain = _s;
    releaseSamples = _r;
}

float ADSR::tick() {
    if (state_ == A) { // attack
        last += attackRate;
        if (last >= attackTarget) {
            last = attackTarget;
            state_ = D;
        }
    } else if (state_ == D) { // decay
        last -= decayRate;
        if (last <= sustain) {
            last = sustain;
            state_ = S;
        }
    } else if (state_ == R) { // release
        last -= releaseRate;
        if (last < 0.0) {
            last = 0.0;
            state_ = IDLE;
        }
    }
    return last;
}

float ADSR::tick(int samples) {
    // TODO optimize
    for (int i = 0; i < samples; i++) {
        tick();
    }
    return last;
}

// AHDSR

void AHDSR::on() {
    // don't reset when retriggered during release
    if (state_ == IDLE) {
        last = 0.0;
    }
    state_ = A;
}

void AHDSR::off() {
    state_ = R;
    releaseRate = last / releaseSamples;
}

void AHDSR::setAHDSR(float _a, float _h, float _d, float _s, float _r) {
    attackRate = attackTarget / _a;
    holdSamples = _h;
    decayRate = (attackTarget - _s) / _d;
    sustain = _s;
    releaseSamples = _r;
}

float AHDSR::tick() {
    if (state_ == A) { // attack
        last += attackRate;
        if (last >= attackTarget) {
            last = attackTarget;
            counter = holdSamples;
            state_ = H;
        }
    } else if (state_ == H) { // hold
        if (counter-- == 0) {
            state_ = D;
        }
    } else if (state_ == D) { // decay
        last -= decayRate;
        if (last <= sustain) {
            last = sustain;
            state_ = S;
        }
    } else if (state_ == R) { // release
        last -= releaseRate;
        if (last < 0.0) {
            last = 0.0;
            state_ = IDLE;
        }
    }
    return last;
}

float AHDSR::tick(int samples) {
    // TODO optimize
    for (int i = 0; i < samples; i++) {
        tick();
    }
    return last;
}

}
