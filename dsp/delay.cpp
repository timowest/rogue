/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "delay.h"

namespace dsp {

// DelayA

DelayA::DelayA() {
    for (uint i = 0; i < length; i++) {
        buffer_[i] = 0.0f;
    }
    last_ = 0.0;
    apInput_ = 0.0;
}

void DelayA::setDelay(float delay) {
    float outPointer = inPoint_ - delay + 1.0; // outPoint chases inpoint
    delay_ = delay;
    while (outPointer < 0) {
        outPointer += length; // modulo maximum length
    }
    outPoint_ = (int) outPointer; // integer part
    if (outPoint_ == length) {
        outPoint_ = 0;
    }
    alpha_ = 1.0 + outPoint_ - outPointer; // fractional part
    if (alpha_ < 0.5) {
        // The optimal range for alpha is about 0.5 - 1.5 in order to
        // achieve the flattest phase delay response.
        outPoint_ += 1;
        if (outPoint_ >= length) {
            outPoint_ -= length;
        }
        alpha_ += 1.0f;
    }
    coeff_ = (1.0 - alpha_) / (1.0 + alpha_); // coefficient for allpass
}

void DelayA::clear() {
    for (uint i = 0; i < length; i++) {
        buffer_[i] = 0.0f;
    }
    last_ = 0.0;
    apInput_ = 0.0;
}

float DelayA::nextOut() {
    if (doNextOut_) {
        // Do allpass interpolation delay.
        nextOutput_ = -coeff_ * last_;
        nextOutput_ += apInput_ + (coeff_ * buffer_[outPoint_]);
        doNextOut_ = false;
    }
    return nextOutput_;
}

float DelayA::process(float input) {
    buffer_[inPoint_++] = input;
    if (inPoint_ == length) {
        inPoint_ = 0;
    }
    last_ = nextOut();
    doNextOut_ = true;
    // Save the allpass input and increment modulo length.
    apInput_ = buffer_[outPoint_++];
    if (outPoint_ == 4096) {
        outPoint_ = 0;
    }
    return last_;
}

}
