/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "oscillator.h"
#include "phase.h"
#include "tables.h"

namespace dsp {

static float polyblep(float t) {
    if (t > 0.0f) {
        return t - (t*t)/2.0f - 0.5f;
    } else {
        return (t*t)/2.0f + t + 0.5f;
    }
}

static float phase_target(float p) {
    if (p >= 0.75f) {
        return 1.0f;
    } else if (p >= 0.25f) {
        return 0.5f;
    } else {
        return 0.0f;
    }
}

static float limit(float x, float max) {
    if (x > max) {
        return fmod(x, max);
    } else {
        return x;
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

float height_diff(float scale, float offset) {
    return limit(scale + offset, 1.0f) - offset;
}

float PhaseShaping::sin2(float p) {
    return sin_.linear(p);
}

void PhaseShaping::sin2(float* output, int samples) {
    float inc = freq / sample_rate;

    for (int i = 0; i < samples; i++) {
        output[i] = sin2(phase);
        phase = fmod(phase + inc, 1.0f);
    }
}

void PhaseShaping::hardsync(float* output, int samples) {
    // bandlimited saw
    // TODO only one discontinuity if (a1 + a0) < 1.0
    float inc = freq / sample_rate;
    float inc2 = a1 * inc;

    for (int i = 0; i < samples; i++) {
        float p2 = gramp(phase, a1, a0);
        float mod = 0.0f;
        if (phase < inc) {                 // start
            float h = height_diff(a1, a0);
            mod = h * polyblep(phase / inc);
        } else if (phase > (1.0f - inc)) { // end
            float h = height_diff(a1, a0);
            mod = h * polyblep( (p2 - h) / inc2);
        } else if (p2 < inc2) {            // start mid
            mod = polyblep(p2 / inc2);
        } else if (p2 > (1.0f - inc2)) {   // end mid
            mod = polyblep( (p2 - 1.0f) / inc2);
        }
        output[i] = gb(p2 - mod);
        phase = fmod(phase + inc, 1.0f);
    }
}

// TODO bandlimiting
void PhaseShaping::softsync(float* output, int samples) {
    // tri
    float inc = freq / sample_rate;

    for (int i = 0; i < samples; i++) {
        output[i] = gb(stri(gtri(phase, a1, a0)));
        phase = fmod(phase + inc, 1.0f);
    }
}

void PhaseShaping::pulse(float* output, int samples) {
    // bandlimited square
    float inc = freq / sample_rate;

    for (int i = 0; i < samples; i++) {
        float p2 = gpulse(phase, width);
        float mod = 0.0f;
        if (phase < inc) {                 // start
            mod = polyblep(phase / inc);
        } else if (phase > (1.0f - inc)) { // end
            mod = polyblep( (phase - 1.0f) / inc);
        } else if (phase < width && phase > (width - inc)) {
            mod = -polyblep( (phase - width) / inc);
        } else if (phase > width && phase < (width + inc)) {
            mod = -polyblep( (phase - width) / inc);
        }
        output[i] = gb(p2 - mod);
        phase = fmod(phase + inc, 1.0f);
    }
}


void PhaseShaping::slope(float* output, int samples) {
    // bandlimited saw
    float inc = freq / sample_rate;
    float inc2 = inc / (1.0f - width);

    for (int i = 0; i < samples; i++) {
        float p2 = gvslope(phase, width);
        float mod = 0.0f;
        if (phase < inc) {               // start
            mod = polyblep(phase / inc);
        } else if (p2 > (1.0f - inc2)) { // end
            mod = polyblep( (p2 - 1.0f) / inc2);
        } else if (phase < width && phase > (width - inc)) {
            mod = width * polyblep( (p2 - width) / inc);
        } else if (phase > width && p2 < inc2) {
            mod = width * polyblep(p2 / inc2);
        }
        output[i] = gb(p2 - mod);
        phase = fmod(phase + inc, 1.0f);
    }
}

// TODO bandlimiting
void PhaseShaping::jp8000_tri(float* output, int samples) {
    float inc = freq / sample_rate;

    for (int i = 0; i < samples; i++) {
        float p2 = gb(gtri(phase, a1, a0));
        output[i] = 2.0 * (p2 - ceil(p2 - 0.5));
        phase = fmod(phase + inc, 1.0f);
    }
}

// TODO bandlimiting
void PhaseShaping::jp8000_supersaw(float* output, int samples) {
    float inc = freq / sample_rate;
    const float m1 = 0.25f / (2.0 * M_PI), m2 = 0.88f / (2.0 * M_PI);

    for (int i = 0; i < samples; i++) {
        float p2 = gripple2(fmod(glin(phase, a1), 1.0f), m1, m2);
        output[i] = sin2(fmod(p2, 1.0f));
        phase = fmod(phase + inc, 1.0f);
    }
}

void PhaseShaping::waveslices(float* output, int samples) {
    // bandlimited ramp -> sin
    float inc = freq / sample_rate;
    float inc2 = a1 * inc;
    float a1_p = limit(a1, 1.0f);
    float diff = phase_target(a1_p) - a1_p;

    for (int i = 0; i < samples; i++) {
        float p2 = limit(glin(phase, a1), 1.0f);
        float mod = 0.0f;
        if (phase < inc) {                 // start
            mod = diff * polyblep(p2 / inc2);
            // TODO simplify this
            if (a1_p > 0.5f) mod *= -1.0f;
            if (p2 - mod < 0.0f) mod -= 1.0f;
        } else if (phase > (1.0f - inc)) { // end
            mod = diff * polyblep( (p2 - a1_p) / inc2);
        }
        output[i] = sin2(fmod(p2 - mod, 1.0f));
        phase = fmod(phase + inc, 1.0f);
    }
}

void PhaseShaping::sinusoids(float* output, int samples) {
    // bandlimited ramp -> sin
    float inc = freq / sample_rate;
    float inc2 = inc / (1.0f - width);
    float diff = phase_target(width) - width;

    for (int i = 0; i < samples; i++) {
        float p2 = gvslope(phase, width);
        float mod = 0.0f;
        if (phase < width && phase > (width - inc)) { // mid end
            mod = diff * polyblep( (p2 - width) / inc);
        } else if (phase > width && p2 < inc2) {      // mid start
            mod = diff * polyblep(p2 / inc2);
            // TODO simplify this
            if (width > 0.5f) mod *= -1.0f;
            if (p2 - mod < 0.0f) mod -= 1.0f;
        }
        output[i] = sin2(fmod(p2 - mod, 1.0f));
        phase = fmod(phase + inc, 1.0f);
    }
}

void PhaseShaping::noise(float* output, int samples) {
    for (int i = 0; i < samples; i++) {
        output[i] =  (2.0f * rand() / (RAND_MAX + 1.0f) - 1.0f);
    }
}

void PhaseShaping::process(float* output, int samples) {
    float inc = freq / sample_rate;

    switch(type) {
    case SIN:
        sin2(output, samples); break;
    case HARD:
        hardsync(output, samples); break;
    case SOFT:
        softsync(output, samples); break;
    case PULSE:
        pulse(output, samples); break;
    case SLOPE:
        slope(output, samples); break;
    case TRI:
        jp8000_tri(output, samples); break;
    case SUPERSAW:
        jp8000_supersaw(output, samples); break;
    case SLICES:
        waveslices(output, samples); break;
    case SINUSOIDS:
        sinusoids(output, samples); break;
    case NOISE:
        noise(output, samples); break;
    }
}

}
