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
    dsp::Virtual virt;
    dsp::AS as;
    dsp::Noise noise;
    float buffer[BUFFER_SIZE];
    float sync[BUFFER_SIZE];
    float prev_level;
    float width_prev = 0.5f;

    void reset() {
        width_prev = 0.5;
        prev_level = 0.0f;
    }

    void setStart(float s) {
        virt.setStart(s);
        as.setStart(s);
        noise.setStart(s);
    }

    void resetPhase() {
        virt.reset();
        as.reset();
        noise.reset();
    }

    void setSamplerate(float r) {
        virt.setSamplerate(r);
        as.setSamplerate(r);
        noise.setSamplerate(r);
    }

    void setModulation(int type, float* _input, float* _input_s, float _pm, bool _sync) {
        if (type < 34) {
            virt.setModulation(_input, _input_s, _pm, _sync);
        } else if (type < 37) {
            as.setModulation(_input, _input_s, _pm, _sync);
        }
    }

    void process(int type, float freq, float wf, float wt, float* buffer, float* sync, int samples) {
        dsp::Oscillator* osc;
        if (type < 34) {
            osc = &virt;
        } else if (type < 37) {
            osc = &as;
            type = type - 34;
        } else {
            osc = &noise;
            type = type - 37;
        }
        osc->setType(type);
        osc->setFreq(freq);
        osc->setWidth(wf, wt);
        osc->process(buffer, sync, samples);
    }
};

struct Filter {
    dsp::MoogFilter moog;
    dsp::StateVariableFilter2 svf;
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
