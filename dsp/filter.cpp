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

void OnePole::setCoefficients(float b0, float a1) {
    b0_ = b0;
    a1_ = a1;
}

void OnePole::setPole(double p) {
    b0_ = (p > 0.0) ? (1.0 - p) : (1.0 + p);
    a1_ = -p;
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
        case 0:
            a0 = k2 / bh;
            a1 = a0 * 2.0;
            a2 = a0;
            b1 = (2.0 * (k2 - 1.0)) / bh;
            b2 = (1.0 - rk + k2) / bh;
            break;
        case 1:
            a0 =  1.0 / bh;
            a1 = -2.0 / bh;
            a2 =  a0;
            b1 = (2.0 * (k2 - 1.0)) / bh;
            b2 = (1.0 - rk + k2) / bh;
            break;
        case 2:
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
    drive_ = 1.0;
    type_ = 0;
    gcomp_ = 0.5;
    dlout_[0] = dlout_[1] = dlout_[2] = dlout_[3] = dlout_[4] = 0.0;
    dlin_[0] = dlin_[1] = dlin_[2] = dlin_[3] = dlin_[4] = 0.0;
}

void MoogFilter::setCoefficients(float freq, float res) {
    wc_ = 2.0f * M_PI * freq / sample_rate_;
    g_ = 0.9892 * wc_ - 0.4342 * pow(wc_, 2) + 0.1381 * pow(wc_, 3) - 0.0202 * pow(wc_, 4);
    gres_ = res * (1.0029 + 0.0526 * wc_ - 0.926 * pow(wc_, 2) + 0.0218 * pow(wc_, 3));
}

#define MOOG_LOOP(x) \
    for (uint i = 0; i < samples; i++) { \
        const float in = input[i]; \
        dlout_[0] = (float) TANH(drive_ * (in - 4.0f * gres_ * (dlout_[4] - gcomp_ * in))); \
        for (uint i = 0; i < 4; i++) { \
            dlout_[i+1] = g_ * (0.3f/1.3f * dlout_[i] + 1.0f/1.3f * dlin_[i] - dlout_[i + 1]) + dlout_[i + 1]; \
            dlin_[i] = dlout_[i]; \
        } \
        output[i] = x; \
    }

void MoogFilter::process(float* input, float* output, int samples) {
    switch(type_) {
    case LP24: // 24dB LP
        MOOG_LOOP(dlout_[4])
        break;
    case LP18: // 18dB LP
        MOOG_LOOP(dlout_[3])
        break;
    case LP12: // 12dB LP
        MOOG_LOOP(dlout_[2])
        break;
    case LP6:  // 6db LP
        MOOG_LOOP(dlout_[1])
        break;
    case HP24: // 24dB HP
        MOOG_LOOP(dlout_[0] - dlout_[4])
        break;
    case BP12: // 12db BP
        MOOG_LOOP(dlout_[4] - dlout_[2])
        break;
    case BP18: // 18/6dB BP
        MOOG_LOOP(dlout_[3] - dlout_[4])
        break;
    case NOTCH:// NOTCH
        MOOG_LOOP((dlout_[3] - dlout_[4]) + 2/3 * dlout_[0])
        break;
    }
}

// MoogFilter 2

static const float VT = 0.026;

static const float TWO_VT = 2.0f * VT;

void MoogFilter2::clear() {
    V1prev = 0.0;
    V2prev = 0.0;
    V3prev = 0.0;
    V4prev = 0.0;
    tV1prev = 0.0;
    tV2prev = 0.0;
    tV3prev = 0.0;
    tV4prev = 0.0;
    dV1prev = 0.0;
    dV2prev = 0.0;
    dV3prev = 0.0;
    dV4prev = 0.0;

    _cutoff = 100;
    _resonance = .05;
    _drive = 0.05;
}

void MoogFilter2::setCoefficients(float cut, float r) {
    _cutoff = cut;

    _x = M_PI * _cutoff / sample_rate_;
    _g = 4.0 * M_PI * VT * _cutoff * (1.0 - _x) / (1.0 + _x);

    _resonance = 3.0 * r;
}

void MoogFilter2::process(float* input, float* output, int samples) {
    const float TWO_SR = 2.0 * sample_rate_;
    const float _out = 1.0f / _drive;

    for (uint i = 0; i < samples; i++) {
        const float in = input[i];

        float dV1 = -_g * (tanh((_drive * in + _resonance * V4prev) / TWO_VT) + tV1prev);
        V1prev += (dV1 + dV1prev) / TWO_SR;
        dV1prev = dV1;
        tV1prev = tanh(V1prev / TWO_VT);

        float dV2 = _g * (tV1prev - tV2prev);
        V2prev += (dV2 + dV2prev) / TWO_SR;
        dV2prev = dV2;
        tV2prev = tanh(V2prev / TWO_VT);

        float dV3 = _g * (tV2prev - tV3prev);
        V3prev += (dV3 + dV3prev) / TWO_SR;
        dV3prev = dV3;
        tV3prev = tanh(V3prev / TWO_VT);

        float dV4 = _g * (tV3prev - tV4prev);
        V4prev += (dV4 + dV4prev) / TWO_SR;
        dV4prev = dV4;
        tV4prev = tanh(V4prev / TWO_VT);

        // Output
        output[i] = _out * V4prev;
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
