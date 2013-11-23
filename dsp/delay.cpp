/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "delay.h"

namespace dsp {

// Delay

Delay::Delay(uint l) {
    buffer = new float[l];
    length = l;
    clear();
}

Delay::~Delay() {
    delete buffer;
}

void Delay::setDelay(uint d) {
    if (inPoint >= d) outPoint = inPoint - d;
    else outPoint = length + inPoint - d;
    delay = d;
}

void Delay::setMax(uint d) {
    if (d > length) {
        delete buffer;
        buffer = new float[d];
        for (uint i = 0; i < d; i++) {
            buffer[i] = 0;
        }
    }
    length = d;
}

void Delay::clear() {
    for (uint i = 0; i < length; i++) {
        buffer[i] = 0;
    }
    last = 0;
}

float Delay::nextOut() {
    return buffer[outPoint];
}

float Delay::process(float in) {
    buffer[inPoint++] = in;
    if (inPoint == length) inPoint = 0;
    last = buffer[outPoint++];
    if (outPoint == length) outPoint = 0;
    return last;
}

// MDelay

MDelay::MDelay(uint l) {
    buffer = new float[l];
    length = l;
    clear();
}

MDelay::~MDelay() {
    delete buffer;
}

void MDelay::setMax(uint d) {
    if (d > length) {
        delete buffer;
        buffer = new float[d];
        for (uint i = 0; i < d; i++) {
            buffer[i] = 0;
        }
    }
    length = d;
}

void MDelay::clear() {
    for (uint i = 0; i < length; i++) {
        buffer[i] = 0;
    }
}

float MDelay::at(int samples) {
    int outPoint = int(inPoint) - samples;
    if (outPoint < 0) outPoint += length;
    return buffer[outPoint];
}

void MDelay::tick(float in) {
    buffer[inPoint++] = in;
    if (inPoint == length) inPoint = 0;
}

// DelayA

DelayA::DelayA(uint l) {
    buffer = new float[l];
    length = l;
    clear();
}

DelayA::~DelayA() {
    //delete buffer; FIXME
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

void DelayA::setMax(uint d) {
    if (d > length) {
        delete buffer;
        buffer = new float[d];
        for (uint i = 0; i < d; i++) {
            buffer[i] = 0;
        }
    }
    length = d;
}

void DelayA::clear() {
    for (uint i = 0; i < length; i++) {
        buffer[i] = 0.0f;
    }
    last_ = 0.0;
    inPoint_ = 0;
    apInput_ = 0.0;
}

float DelayA::nextOut() {
    if (doNextOut_) {
        // Do allpass interpolation delay.
        nextOutput_ = -coeff_ * last_;
        nextOutput_ += apInput_ + (coeff_ * buffer[outPoint_]);
        doNextOut_ = false;
    }
    return nextOutput_;
}

float DelayA::process(float input) {
    buffer[inPoint_++] = input;
    if (inPoint_ == length) {
        inPoint_ = 0;
    }
    last_ = nextOut();
    doNextOut_ = true;
    // Save the allpass input and increment modulo length.
    apInput_ = buffer[outPoint_++];
    if (outPoint_ == length) {
        outPoint_ = 0;
    }
    return last_;
}

// DelayL

DelayL::DelayL(uint l) {
    buffer = new float[l];
    length = l;
    inPoint_ = 0;
    clear();
}

DelayL::~DelayL() {
    delete buffer;
}

void DelayL::setDelay(float delay) {
    float outPointer = inPoint_ - delay;  // read chases write
    delay_ = delay;

    while (outPointer < 0) {
        outPointer += length; // modulo maximum length
    }

    outPoint_ = (uint) outPointer;   // integer part
    if (outPoint_ == length) outPoint_ = 0;
    alpha_ = outPointer - outPoint_; // fractional part
    omAlpha_ = (float) 1.0 - alpha_;
}

void DelayL::setMax(uint d) {
    if (d > length) {
        delete buffer;
        buffer = new float[d];
        for (uint i = 0; i < d; i++) {
            buffer[i] = 0;
        }
    }
    length = d;
}

void DelayL::clear() {
    for (uint i = 0; i < length; i++) {
        buffer[i] = 0.0f;
    }
    last_ = 0.0;
}

float DelayL::nextOut() {
    if (doNextOut_) {
        // First 1/2 of interpolation
        nextOutput_ = buffer[outPoint_] * omAlpha_;
        // Second 1/2 of interpolation
        if (outPoint_+ 1 < length)
          nextOutput_ += buffer[outPoint_+1] * alpha_;
        else
          nextOutput_ += buffer[0] * alpha_;
        doNextOut_ = false;
    }
    return nextOutput_;
}

float DelayL::process(float input) {
    buffer[inPoint_++] = input;
    if (inPoint_ == length) {
        inPoint_ = 0;
    }
    last_ = nextOut();
    doNextOut_ = true;
    if (++outPoint_ == length) {
        outPoint_ = 0;
    }
    return last_;
}

}
