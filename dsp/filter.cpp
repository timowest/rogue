/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "filter.h"

#include <algorithm>
#include <math.h>
#include "tables.h"
#include "types.h"

#define TANH(x) tanh_.linear(x)

namespace dsp {

// DCBlocker

void DCBlocker::clear() {
    x1 = y1 = 0.0f;
}

void DCBlocker::setSamplerate(float r) {
    R = 1.0f - (M_PI * 2.0f * 20.0f / r);
}

void DCBlocker::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        // y(n) = x(n) - x(n - 1) + R * y(n - 1)
        float y = input[i] - x1 + R * y1;
        x1 = input[i];
        y1 = y;
        output[i] = y;
    }
}

// OnePole

void OnePole::clear(){
    last_ = 0.0;
}

float OnePole::getLast() {
    return last_;
}

void OnePole::setCoefficients(float b0, float a1) {
    b0_ = b0;
    a1_ = a1;
}

void OnePole::setPole(double p) {
    b0_ = (p > 0.0) ? (1.0 - p) : (1.0 + p);
    a1_ = -p;
}

void OnePole::setLowpass(double fc) {
    a1_ = exp(-2.0 * M_PI * fc);
    b0_ = 1.0 - a1_;
}

void OnePole::setHighpass(double fc) {
    a1_ = -exp(-2.0 * M_PI * (0.5 - fc));
    b0_ = 1.0 + a1_;
}

float OnePole::process(float input) {
    last_ = b0_ * input - a1_ * last_;
    return last_;
}

void OnePole::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        last_ = b0_ * input[i] - a1_ * last_;
        output[i] = last_;
    }
}

// OneZero

void OneZero::clear() {
    last_ = prevIn_ = 0.0;
}

void OneZero::setCoefficients(float b0, float b1) {
    b0_ = b0;
    b1_ = b1;
}

void OneZero::setZero(float z) {
    b0_ = (z > 0.0) ? (1.0 + z) : (1.0 - z);
    b1_ = -z * b0_;
}

float OneZero::process(float input) {
    last_ = b0_ * input + b1_ * prevIn_;
    prevIn_ = input;
    return last_;
}

void OneZero::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        last_ = b0_ * input[i] + b1_ * prevIn_;
        prevIn_ = input[i];
        output[i] = last_;
    }
}

// PoleZero

void PoleZero::clear() {
    last_ = prevIn_ = 0.0;
}

void PoleZero::setCoefficients(float b0, float b1, float a1) {
    b0_ = b0;
    b1_ = b1;
    a1_ = a1;
}

void PoleZero::setAllpass(float coefficient) {
    b0_ = coefficient;
    b1_ = 1.0;
    a0_ = 1.0; // just in case
    a1_ = coefficient;
}

void PoleZero::setBlockZero(float thePole) {
    b0_ = 1.0;
    b1_ = -1.0;
    a0_ = 1.0; // just in case
    a1_ = -thePole;
}

float PoleZero::process(float input) {
    last_ = b0_ * input + b1_ * prevIn_ - a1_ * last_;
    prevIn_ = input;
    return last_;
}

void PoleZero::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        last_ = b0_ * input[i] + b1_ * prevIn_ - a1_ * last_;
        prevIn_ = input[i];
        output[i] = last_;
    }
}

// TwoPole

void TwoPole::clear() {
    last_ = last__ = 0.0;
}

void TwoPole::setCoefficients(float b0, float a1, float a2) {
    b0_ = b0;
    a1_ = a1;
    a2_ = a2;
}

float TwoPole::process(float input) {
    float temp = last_;
    last_ = b0_ * input - a1_ * last_ - a2_ * last__;
    last__ = temp;
    return last_;
}

void TwoPole::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        float temp = last_;
        last_ = b0_ * input[i] - a1_ * last_ - a2_ * last__;
        last__ = temp;
        output[i] = last_;
    }
}

// TwoZero

void TwoZero::clear() {
    last_ = prevIn_ = prevIn__ = 0.0;
}

void TwoZero::setCoefficients(float b0, float b1, float b2) {
    b0_ = b0;
    b1_ = b1;
    b2_ = b2;
}

float TwoZero::process(float input) {
    last_ = b0_ * input + b1_ * prevIn_ + b2_ * prevIn__;
    prevIn__ = prevIn_;
    prevIn_ = input;
    return last_;
}

void TwoZero::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        last_ = b0_ * input[i] + b1_ * prevIn_ + b2_ * prevIn__;
        prevIn__ = prevIn_;
        prevIn_ = input[i];
        output[i] = last_;
    }
}

// BiQuad

void BiQuad::clear() {
    last_ = last__ = prevIn_ = prevIn__ = 0.0;
}

void BiQuad::setCoefficients(float b0, float b1, float b2, float a1, float a2) {
    b0_ = b0;
    b1_ = b1;
    b2_ = b2;
    a1_ = a1;
    a2_ = a2;
}

float BiQuad::process(float input) {
    float temp = last_;
    last_ = b0_ * input + b1_ * prevIn_ + b2_ * prevIn__;
    last_ -= a1_ * temp + a2_ * last__;
    prevIn__ = prevIn_;
    prevIn_ = input;
    last__ = temp;
    return last_;
}

void BiQuad::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        float temp = last_;
        last_ = b0_ * input[i] + b1_ * prevIn_ + b2_ * prevIn__;
        last_ -= a1_ * temp + a2_ * last__;
        prevIn__ = prevIn_;
        prevIn_ = input[i];
        last__ = temp;
        output[i] = last_;
    }
}

// AmSynth

void AmSynthFilter::clear() {
    d1 = 0.0;
    d2 = 0.0;
    d3 = 0.0;
    d4 = 0.0;
}

void AmSynthFilter::setCoefficients(float f, float r) {
    freq_ = std::min(f, float(sample_rate_ / 2.0) * 0.99f); // filter is unstable at PI
    freq_ = std::max(freq_, 10.0f);
    res_ = r;
}

void AmSynthFilter::process(float* input, float* output, int samples) {
    const double w = (freq_ / sample_rate_); // cutoff freq [ 0 <= w <= 0.5 ]
    const double r = std::max(0.001, 2.0 * (1.0 - res_)); // r is 1/Q (sqrt(2) for a butterworth response)

    const double k = tan(w * M_PI);
    const double k2 = k * k;
    const double rk = r * k;
    const double bh = 1.0 + rk + k2;

    double a0, a1, a2, b1, b2;

    switch (type_ % 3) {
        case 0: // low
            a0 = k2 / bh;
            a1 = a0 * 2.0;
            a2 = a0;
            b1 = (2.0 * (k2 - 1.0)) / bh;
            b2 = (1.0 - rk + k2) / bh;
            break;
        case 1: // high
            a0 =  1.0 / bh;
            a1 = -2.0 / bh;
            a2 =  a0;
            b1 = (2.0 * (k2 - 1.0)) / bh;
            b2 = (1.0 - rk + k2) / bh;
            break;
        case 2: // band
            a0 =  rk / bh;
            a1 =  0.0;
            a2 = -rk / bh;
            b1 = (2.0 * (k2 - 1.0)) / bh;
            b2 = (1.0 - rk + k2) / bh;
            break;
    }

    if (type_ < 3) {
        for (int i = 0; i < samples; i++) {
            float y, x = input[i];

            y  =      (a0 * x) + d1;
            d1 = d2 + (a1 * x) - (b1 * y);
            d2 =      (a2 * x) - (b2 * y);

            x = y;

            y  =      (a0 * x) + d3;
            d3 = d4 + (a1 * x) - (b1 * y);
            d4 =      (a2 * x) - (b2 * y);

            output[i] = y;
        }
    } else {
        for (int i = 0; i < samples; i++) {
            float y, x = input[i];

            y  =      (a0 * x) + d1;
            d1 = d2 + (a1 * x) - (b1 * y);
            d2 =      (a2 * x) - (b2 * y);

            output[i] = y;
        }
    }
}

// MoogFilter

void MoogFilter::clear() {
    f = 0; pc = 0; q = 0;
    bf0 = 0; bf1 = 0; bf2 = 0; bf3 = 0; bf4 = 0;
    t1 = 0; t2 = 0;
}

void MoogFilter::setCoefficients(float f_, float r_) {
    float frequency = f_ / sample_rate;
    float resonance = r_;

    if (frequency < 0) frequency = 0;
    if (frequency > 0.6) frequency = 0.6;

    q = 1.0f - frequency;
    pc = frequency + 0.8f * frequency * q;
    f = pc + pc - 1.0f;
    q = resonance * (1.0f + 0.5f * q * (1.0f - q + 5.6f * q * q));
}

void MoogFilter::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        float in = input[i];

        in -= q * bf4; //feedback
        t1 = bf1;  bf1 = (in + bf0) * pc - bf1 * f;
        t2 = bf2;  bf2 = (bf1 + t1) * pc - bf2 * f;
        t1 = bf3;  bf3 = (bf2 + t2) * pc - bf3 * f;
        bf4 = (bf3 + t1) * pc - bf4 * f;
        bf4 = bf4 - bf4 * bf4 * bf4 * 0.166667f;    //clipping
        bf0 = in;

        // Lowpass  output:  bf4
        // Highpass output:  in - bf4;
        // Bandpass output:  3.0f * (bf3 - bf4);
        output[i] = bf4;
    }
}

// StateVariableFilter

void StateVariableFilter::clear() {
    drive = 0.0;
    notch = low = high = band = 0.0;
    out = &low;
}

void StateVariableFilter::setType(int type) {
    switch(type) {
    case LP:
        out = &low; break;
    case HP:
        out = &high; break;
    case BP:
        out = &band; break;
    case NOTCH:
        out = &notch; break;
    }
}

void StateVariableFilter::setCoefficients(float fc, float res) {
    freq = 2.0 * sinf(M_PI * std::min(0.25, fc / (sample_rate * 2.0)));
    damp = std::min(2.0*(1.0 - pow(res, 0.25)), std::min(2.0, 2.0 / freq - freq * 0.5));
}

void StateVariableFilter::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        // TODO figure out better upsampling method
        const float in = input[i];
        notch = in - damp * band;
        low   = low + freq * band;
        high  = notch - low;
        band  = freq * high + band - drive * band * band * band;
        output[i] = 0.5 * *out;

        notch = in - damp * band;
        low   = low + freq * band;
        high  = notch - low;
        band  = freq * high + band - drive * band * band * band;
        output[i] += 0.5 * *out;
    }
}

// StateVariableFilter 2

void StateVariableFilter2::clear() {
    v0z = v1 = v2 = 0.0f;
}

void StateVariableFilter2::setCoefficients(float fc, float res) {
    float g = tan(M_PI * fc / sample_rate);
    //float damping = 1.0f / res;
    //k = damping;
    k = 1.0 - 0.99 * res;
    float ginv = g / (1.0f + g * (g + k));
    g1 = ginv;
    g2 = 2.0f * (g + k) * ginv;
    g3 = g * ginv;
    g4 = 2.0f * ginv;
}

#define SVF2_LOOP(x) \
    for (uint i = 0; i < samples; i++) { \
        float v0 = input[i]; \
        float v1z = v1; \
        float v2z = v2; \
        float v3 = v0 + v0z - 2.0 * v2z; \
        v1 += g1 * v3 - g2 * v1z; \
        v2 += g3 * v3 + g4 * v1z; \
        v0z = v0; \
        output[i] = x; \
    }

void StateVariableFilter2::process(float* input, float* output, int samples) {
    switch (type) {
    case LP:
        SVF2_LOOP(v2);
        break;
    case BP:
        SVF2_LOOP(v1);
        break;
    case HP:
        SVF2_LOOP(v0 - k * v1 - v2);
        break;
    case NOTCH:
        SVF2_LOOP(v0 - k * v1);
        break;
    }
}

// CombFilter


void CombFilter::clear() {
    delay.clear();
}

void CombFilter::setCoefficients(float _fc, float _amount) {
    delay.setDelay((1.0 / _fc) * sample_rate);
    fc = _fc;
    amount = _amount;
}

void CombFilter::process(float* input, float* output, int samples) {
    for (uint i = 0; i < samples; i++) {
        output[i] = input[i] + amount * delay.process(input[i]);
    }
}

}
