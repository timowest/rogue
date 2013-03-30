/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "oscillator.h"
#include "phase.h"

namespace dsp {

// PhaseShaping

void PhaseShaping::clear() {
    phase = 0.0;
    freq = 440.0;
    type = SIN;
}

void PhaseShaping::reset() {
    phase = 0.0;
}

float PhaseShaping::sin2(float in) {
    // TODO use lookup table
    return sin(2.0 * M_PI * in);
}

float PhaseShaping::hardsync(float in) {
    return gb(gramp(in, a1, a0));
}

float PhaseShaping::softsync(float in) {
    return gb(stri(gtri(in, a1, a0)));
}

float PhaseShaping::pulse(float in) {
    return gb(gpulse(in, a1));
}

float PhaseShaping::slope(float in) {
    return gb(gvslope(in, a1));
}

float PhaseShaping::jp8000_tri(float in) {
    float p = gb(gtri(in, a1, a0));
    return 2.0 * (p - ceil(p - 0.5));
}

float PhaseShaping::jp8000_supersaw(float in) {
    float m1, m2; // TODO
    float p = gripple2(glin(in, a1), m1, m2);
    return sin2(p);
}

float PhaseShaping::waveslices(float in) {
    return sin2(glin(in, a1));
}

float PhaseShaping::sinusoids(float in) {
    return sin2(gvslope(in, a1));
}

float PhaseShaping::noise() {
    return (double) (2.0 * rand() / (RAND_MAX + 1.0) - 1.0);
}

void PhaseShaping::process(float* output, int samples) {
    float inc = sampleRate / freq;

    for (int i = 0; i < samples; i++) {
        phase += inc;
        if (phase > 1.0) {
            phase -= 1.0;
        }

        // TODO type switch

        // sin
        output[i] = sin2(phase);
        // hard sync
        output[i] = hardsync(phase);
        // soft sync
        output[i] = softsync(phase);
        // pulse
        output[i] = pulse(phase);
        // slope
        output[i] = slope(phase);
        // jp8000 tri
        output[i] = jp8000_tri(phase);
        // jp8000 supersaw
        output[i] = jp8000_supersaw(phase);
        // waveslices
        output[i] = waveslices(phase);
        // sinusoids
        output[i] = sinusoids(phase);
        // noise
        output[i] = noise();
    }
}

}
