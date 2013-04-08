/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "oscillator.h"
#include "phase.h"
#include "tables.h"

namespace dsp {

/**
 * @param t phase from 0 to 1
 * @param dt increment (freq / sample_rate)
 *
 * based on http://www.kvraudio.com/forum/viewtopic.php?p=5270781
 */
double poly_blep(double t, double dt) {
    if (t < dt) { // 0 <= t < 1
        t /= dt;
        // 2 * (t - t^2/2 - 0.5)
        return t+t - t*t - 1.;
    } else if (t > 1.0 - dt) { // -1 < t < 0
        t = (t - 1.0) / dt;
        // 2 * (t^2/2 + t + 0.5)
        return t*t + t+t + 1.;
    } else { // 0 otherwise
        return 0.0;
    }
}

// PhaseShaping

void PhaseShaping::clear() {
    phase = 0.0;
    freq = 440.0;
    type = SIN;
}

void PhaseShaping::reset() {
    phase = 0.0;
}

void PhaseShaping::setParams(float a1_, float a0_) {
    a1 = a1_;
    a0 = a0_;
}

float PhaseShaping::sin2(float in) {
    return sin_.linear(in);
}

float PhaseShaping::hardsync(float in) {
    // saw
    return gb(gramp(in, a1, a0));
}

float PhaseShaping::softsync(float in) {
    // tri
    return gb(stri(gtri(in, a1, a0)));
}

float PhaseShaping::pulse(float in) {
    // square
    return gb(gpulse(in, a1));
}

float PhaseShaping::slope(float in) {
    // saw
    return gb(gvslope(in, a1));
}

float PhaseShaping::jp8000_tri(float in) {
    // tri
    float p = gb(gtri(in, a1, a0));
    return 2.0 * (p - ceil(p - 0.5));
}

float PhaseShaping::jp8000_supersaw(float in) {
    // saw
    float m1, m2; // TODO
    float p = gripple2(glin(in, a1), m1, m2);
    return sin2(p);
}

float PhaseShaping::waveslices(float in) {
    // saw
    return sin2(glin(in, a1));
}

float PhaseShaping::sinusoids(float in) {
    // saw
    return sin2(gvslope(in, a1));
}

float PhaseShaping::noise() {
    return (2.0 * rand() / (RAND_MAX + 1.0) - 1.0);
}

#define PHASE_SHAPING_LOOP(x) \
    for (int i = 0; i < samples; i++) { \
        output[i] = x; \
        phase = fmod(phase + inc, 1.0); \
    } \
    break

void PhaseShaping::process(float* output, int samples) {
    float inc = freq / sample_rate;

    switch(type) {
    case SIN:
        PHASE_SHAPING_LOOP(sin2(phase));
    case HARD:
        PHASE_SHAPING_LOOP(hardsync(phase));
    case SOFT:
        PHASE_SHAPING_LOOP(softsync(phase));
    case PULSE:
        PHASE_SHAPING_LOOP(pulse(phase));
    case SLOPE:
        PHASE_SHAPING_LOOP(slope(phase));
    case TRI:
        PHASE_SHAPING_LOOP(jp8000_tri(phase));
    case SUPERSAW:
        PHASE_SHAPING_LOOP(jp8000_supersaw(phase));
    case SLICES:
        PHASE_SHAPING_LOOP(waveslices(phase));
    case SINUSOIDS:
        PHASE_SHAPING_LOOP(sinusoids(phase));
    case NOISE:
        for (int i = 0; i < samples; i++) {
           output[i] = noise();
        }
    }
}

}
