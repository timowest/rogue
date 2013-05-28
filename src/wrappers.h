/*
 * rogue - multimode synth
 *
 * contains dsp element wrappers and voice class
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_WRAPPERS_H
#define ROGUE_WRAPPERS_H

#include "dsp.h"

namespace rogue {

struct Osc {
    dsp::VA va;
    dsp::PD pd;
    dsp::EL el;
    dsp::AS as;
    dsp::Noise noise;
    float buffer[BUFFER_SIZE];
    float prev_level;
    float width_prev = 0.5f;
    dsp::Oscillator* oscs[5];

    void reset() {
        width_prev = 0.5;
        prev_level = 0.0f;
    }

    void resetPhase() {
        for (int i = 0; i < 5; i++) {
            oscs[i]->reset();
        }
    }

    void setSamplerate(float r) {
        oscs[0] = &va;
        oscs[1] = &pd;
        oscs[2] = &el;
        oscs[3] = &as;
        oscs[4] = &noise;

        for (int i = 0; i < 5; i++) {
            oscs[i]->setSamplerate(r);
        }
    }

    void process(int type, float freq, float t, float wf, float wt, float* buffer, int samples) {
        dsp::Oscillator* osc;
        if (type < 3) {
            osc = &va;
        } else if (type < 12) {
            osc = &pd;
            type = type - 3;
        } else if (type < 23) {
            osc = &el;
            type = type - 12;
        } else if (type < 26) {
            osc = &as;
            type = type - 23;
        } else {
            osc = &noise;
            type = type - 26;
        }
        osc->setType(type);
        osc->setFreq(freq);
        osc->setParams(t, wf, wt);
        osc->process(buffer, samples);
    }
};

struct Filter {
    dsp::MoogFilter moog;
    dsp::StateVariableFilter svf;
    float buffer[BUFFER_SIZE];
    float prev_level;
    float key_vel_to_f;

    void reset() {
        prev_level = 0.0f;
    }

    void setSamplerate(float r) {
        moog.setSamplerate(r);
        svf.setSamplerate(r);
    }
};

struct LFO {
    dsp::LFO lfo;
    float current, last;

    void on() {}

    void off() {}

    void reset() {
        current = 0.0f;
        last = 0.0f;
    }

    void setSamplerate(float r) {
        lfo.setSamplerate(r);
    }
};

struct Env {
    dsp::AHDSR env;
    float current, last;

    void on() { env.on(); }

    void off() { env.off(); }

    void reset() {
        current = 0.0f;
        last = 0.0f;
    }
};

}

#endif
