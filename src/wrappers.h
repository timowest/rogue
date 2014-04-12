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
    dsp::SuperWave superWave;

    float buffer[BUFFER_SIZE];
    float sync[BUFFER_SIZE];
    float prev_level;
    float width_prev = 0.5f;
    float freq_prev = -1.0;

    void reset() {
        width_prev = 0.5;
        prev_level = 0.0f;
        freq_prev = -1.0;
    }

    void setStart(float s) {
        virt.setStart(s);
        as.setStart(s);
        noise.setStart(s);
        superWave.setStart(s);
    }

    void resetPhase() {
        virt.reset();
        as.reset();
        noise.reset();
        superWave.reset();
    }

    void setSamplerate(float r) {
        virt.setSamplerate(r);
        as.setSamplerate(r);
        noise.setSamplerate(r);
        superWave.setSamplerate(r);
    }

    void setModulation(int type, float* _input, float* _input_s, float _pm, bool _sync) {
        if (type < 29) {
            virt.setModulation(_input, _input_s, _pm, _sync);
        } else if (type < 32) {
            as.setModulation(_input, _input_s, _pm, _sync);
        }
    }

    void process(int type, float ff, float ft, float wf, float wt, float* buffer, float* sync, int samples) {
        dsp::Oscillator* osc;
        if (type < 29) {
            osc = &virt;
        } else if (type < 32) {
            osc = &as;
            type -= 29;
        } else if (type < 36) {
            osc = &superWave;
            type -= 32;
        } else {
            osc = &noise;
            type -= 36;
        }
        osc->setType(type);
        osc->setFreq(ff, ft);
        osc->setWidth(wf, wt);
        osc->process(buffer, sync, samples);
    }
};

struct Filter {
    dsp::AmSynthFilter am;
    dsp::MoogFilter moog;
    dsp::StateVariableFilter2 svf;
    dsp::CombFilter comb;
    float buffer[BUFFER_SIZE];
    float prev_level;
    float key_vel_to_f;

    Filter() {
        am.clear();
        moog.clear();
        svf.clear();
        comb.clear();
    }

    void reset() {
        prev_level = 0.0f;
        am.clear();
        moog.clear();
        svf.clear();
        comb.clear();
    }

    void setSamplerate(float r) {
        am.setSamplerate(r);
        moog.setSamplerate(r);
        svf.setSamplerate(r);
        comb.setSamplerate(r);
    }
};

struct LFO {
    dsp::LFO lfo;
    float current, last;

    void on() {
        lfo.reset();
    }

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

    void on() {
        env.on();
    }

    void off() {
        env.off();
    }

    void reset() {
        current = 0.0f;
        last = 0.0f;
    }
};

}

#endif
