/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "oscillator.h"
#include "phase.h"
#include "tables.h"

namespace dsp {

float polyblep(float t) {
    if (t > 0.0f) {
        return t - (t*t)/2.0f - 0.5f;
    } else {
        return (t*t)/2.0f + t + 0.5f;
    }
}

// PhaseShaping

void PhaseShaping::clear() {
    phase = 0.0f;
    freq = 440.0f;
    type = SIN;
    width = 0.5f;
    a1 = 1.0f;
    a0 = 0.0f;
}

void PhaseShaping::reset() {
    phase = 0.0;
}

void PhaseShaping::setWidth(float w) {
    width = w;
}

void PhaseShaping::setParams(float a1_, float a0_) {
    a1 = a1_;
    a0 = a0_;
}

float PhaseShaping::sin2(float p) {
    return sin_.linear(p);
}

float height_diff(float scale, float offset) {
    float end = scale + offset;
    if (end > 1.0f) {
        end = fmod(end, 1.0f);
    }
    return end - offset;
}

float PhaseShaping::hardsync(float inc, float p) {
    // bandlimited saw
    // TODO only one discontinuity if (a1 + a0) < 1.0
    float inc2 = a1 * inc;
    float p2 = gramp(p, a1, a0);
    float mod = 0.0f;
    if (p < inc) {                  // start
        float h = height_diff(a1, a0);
        mod = h * polyblep(p / inc);
    } else if (p > (1.0f - inc)) {  // end
        float h = height_diff(a1, a0);
        mod = h * polyblep( (p2 - h) / inc2);
    } else if (p2 < inc2) {         // start mid
        mod = polyblep(p2 / inc2);
    } else if (p2 > (1.0f - inc2)) {// end mid
        mod = polyblep( (p2 - 1.0f) / inc2);
    }
    return gb(p2 - mod);
}


float PhaseShaping::softsync(float p) {
    // tri
    // TODO bandlimiting
    return gb(stri(gtri(p, a1, a0)));
}

float PhaseShaping::pulse(float inc, float p) {
    // bandlimited square
    float p2 = gpulse(p, width);
    float mod = 0.0f;
    if (p < inc) {                 // start
        mod = polyblep(p / inc);
    } else if (p > (1.0f - inc)) { // end
        mod = polyblep( (p - 1.0f) / inc);
    } else if (p < width && p > (width - inc)) { // mid end
        mod = -polyblep( (p - width) / inc);
    } else if (p > width && p < (width + inc)) { // mid start
        mod = -polyblep( (p - width) / inc);
    }
    return gb(p2 - mod);
}

float PhaseShaping::slope(float inc, float p) {
    // bandlimited saw
    float inc2 = inc / (1.0f - width);
    float p2 = gvslope(p, width);
    float mod = 0.0f;
    if (p < inc) {                 // start
        mod = polyblep(p / inc);
    } else if (p2 > (1.0f - inc2)) { // end
        mod = polyblep( (p2 - 1.0f) / inc2);
    } else if (p < width && p > (width - inc)) {
        mod = width * polyblep( (p2 - width) / inc);
    } else if (p > width && p2 < inc2) {
        mod = width * polyblep(p2 / inc2);
    }
    return gb(p2 - mod);
}

float PhaseShaping::jp8000_tri(float p) {
    // tri
    // TODO bandlimiting
    float p2 = gb(gtri(p, a1, a0));
    return 2.0 * (p2 - ceil(p2 - 0.5));
}

float PhaseShaping::jp8000_supersaw(float p) {
    // saw
    // TODO bandlimiting
    float m1, m2;
    float p2 = gripple2(glin(p, a1), m1, m2);
    return sin2(fmod(p2, 1.0f));
}

float PhaseShaping::waveslices(float inc, float p) {
    // TODO fix polyblep usage
    // bandlimited ramp -> sin
    float inc2 = a1 * inc;
    float p2 = glin(p, a1);
    float mod = 0.0f;
    // XXX same correction as in hardsync
    if (p2 < inc2) {                // start
        mod = a1 * polyblep(p2 / inc2);
    } else if (p2 > (a1 - inc2)) {  // end
        mod = a1 * polyblep( (p2 - a1) / inc2);
    }
    return sin2(fmod(p2 - mod, 1.0f));
}

float PhaseShaping::sinusoids(float inc, float p) {
    // TODO fix polyblep usage
    // bandlimited ramp -> sin
    float inc2 = inc / (1.0f - width);
    float p2 = gvslope(p, width);
    float mod = 0.0f;
    // XXX same correction as in slope
    if (p < inc) {                 // start
        mod = polyblep(p / inc);
    } else if (p2 > (1.0f - inc2)) { // end
        mod = polyblep( (p2 - 1.0f) / inc2);
    } else if (p < width && p > (width - inc)) {
        mod = width * polyblep( (p2 - width) / inc);
    } else if (p > width && p2 < inc2) {
        mod = width * polyblep(p2 / inc2);
    }
    return sin2(fmod(p2 - mod, 1.0f));
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

    // TODO move loops to type specific functions for better optimizations
    switch(type) {
    case SIN:
        PHASE_SHAPING_LOOP(sin2(phase));
    case HARD:
        PHASE_SHAPING_LOOP(hardsync(inc, phase));
    case SOFT:
        PHASE_SHAPING_LOOP(softsync(phase));
    case PULSE:
        PHASE_SHAPING_LOOP(pulse(inc, phase));
    case SLOPE:
        PHASE_SHAPING_LOOP(slope(inc, phase));
    case TRI:
        PHASE_SHAPING_LOOP(jp8000_tri(phase));
    case SUPERSAW:
        PHASE_SHAPING_LOOP(jp8000_supersaw(phase));
    case SLICES:
        PHASE_SHAPING_LOOP(waveslices(inc, phase));
    case SINUSOIDS:
        PHASE_SHAPING_LOOP(sinusoids(inc, phase));
    case NOISE:
        for (int i = 0; i < samples; i++) {
           output[i] = noise();
        }
    }
}

}
