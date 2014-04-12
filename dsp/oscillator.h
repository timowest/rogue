/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_OSCILLATOR_H
#define DSP_OSCILLATOR_H

#define SEMITONE 1.05946f

#include <math.h>
#include "filter.h"

namespace dsp {

/**
 * abstract oscillator class
 */
class Oscillator {
  protected:
    float ff = 440.0f, ft = 440.0f, sample_rate, phase = 0.0f, phase_ = 0.0f;
    float start = 0.0f, wf = 0.5f, wt = 0.5f;
    int type = 0;

    // audio modulation
    float* input;
    float* input_sync;
    float pm = 0.0f;
    bool sync = false;

  public:
    void setType(int t) { type = t; }
    virtual void setSamplerate(float r) { sample_rate = r; }

    void setFreq(float _ff, float _ft) { 
        ff = _ff;
        ft = _ft;
    }

    /** phase modulation */
    float pmod(float phase, int i) {
        float phase2 = phase + pm * input[i];
        if (phase2 < 0.0f) {
            phase2 = 1.0f - fmod(fabs(phase2), 1.0f);
        } else if (phase2 >= 1.0f) {
            phase2 = fmod(phase2, 1.0f);
        }
        return phase2;
    }

    void setModulation(float* _input, float* _input_s, float _pm, bool _sync) {
        input = _input;
        input_sync = _input_s;
        pm = _pm;
        sync = _sync;
    }

    void setStart(float _s) {
        start = _s;
    }

    void setWidth(float _wf, float _wt) {
        wf = _wf;
        wt = _wt;
    }

    virtual void clear() {
        phase = start;
        phase_ = start;
        ff = ft = 440.0;
        type = 0;

        wf = wt = 0.5;
    }

    virtual void reset() {
        phase = start;
        phase_ = start;
    }

    virtual void process(float* output, float* out_sync, int samples) = 0;
};

/** Virtual
 *  Analog - Vintage waveform models
 *  Phase Distortion - Casio CZ sounds
 *  Electronic - various
 *  FM - OPL FM sounds
 */

class Virtual : public Oscillator {

    // 29
    enum {VA_SAW, VA_TRI_SAW, VA_PULSE,
          PD_SAW, PD_SQUARE, PD_PULSE, PD_DOUBLE_SINE, PD_SAW_PULSE, PD_RES1, PD_RES2, PD_RES3, PD_HALF_SINE,
          EL_SAW, EL_DOUBLE_SAW, EL_TRI, EL_PULSE, EL_PULSE_SAW, EL_SLOPE, EL_ALPHA1, EL_ALPHA2, EL_EXP,
          FM1, FM2, FM3, FM4, FM5, FM6, FM7, FM8
    };

    float prev = 0.0f;

    AmSynthFilter filter;

  public:
    void setSamplerate(float r) {
        Oscillator::setSamplerate(r);
        filter.setSamplerate(r);
    }

    void clear();
    void reset();

    // VA
    void va_saw(float* output, float* sync, int samples);
    void va_tri_saw(float* output, float* sync, int samples);
    void va_pulse(float* output, float* sync, int samples);

    // PD
    void pd_saw(float* output, float* sync, int samples);
    void pd_square(float* output, float* sync, int samples);
    void pd_pulse(float* output, float* sync, int samples);
    void pd_double_sine(float* output, float* sync, int samples);
    void pd_saw_pulse(float* output, float* sync, int samples);
    void pd_res1(float* output, float* sync, int samples);
    void pd_res2(float* output, float* sync, int samples);
    void pd_res3(float* output, float* sync, int samples);
    void pd_half_sine(float* output, float* sync, int samples);

    // EL
    void el_saw(float* output, float* sync, int samples);
    void el_double_saw(float* output, float* sync, int samples);
    void el_tri(float* output, float* sync, int samples);
    void el_pulse(float* output, float* sync, int samples);
    void el_pulse_saw(float* output, float* sync, int samples);
    void el_slope(float* output, float* sync, int samples);
    void el_alpha1(float* output, float* sync, int samples);
    void el_alpha2(float* output, float* sync, int samples);
    void el_exp(float* output, float* sync, int samples);

    // FM
    void fm1(float* output, float* sync, int samples);
    void fm2(float* output, float* sync, int samples);
    void fm3(float* output, float* sync, int samples);
    void fm4(float* output, float* sync, int samples);
    void fm5(float* output, float* sync, int samples);
    void fm6(float* output, float* sync, int samples);
    void fm7(float* output, float* sync, int samples);
    void fm8(float* output, float* sync, int samples);

    void process(float* output, float* sync, int samples);

};

// TODO JP8000 tri mod

/**
 * Additive Synthesis
 */
class AS : public Oscillator {

    enum {SAW, SQUARE, TRIANGLE};

    float phases[20];

  public:
    void clear();
    void reset();
    void saw(float* output, float* sync, int samples);
    void square(float* output, float* sync, int samples);
    void triangle(float* output, float* sync, int samples);
    void process(float* output, float* sync, int samples);
};

/**
 * Aliasing Supersaw and Supersquare
 *
 * 7 oscillators, width controls detune amount (max: +/-1 semitone)
 *
 * Uses ideas and values from How to Emulate the Supersaw, 2010 Adam Szabo
 */
class SuperWave : public Oscillator {

    enum {SAW, SQUARE, SAW2, SQUARE2};

    float phases[7];

    AmSynthFilter filter;

  public:
    void setSamplerate(float r) {
        Oscillator::setSamplerate(r);
        filter.setSamplerate(r);
        filter.setType(1);
    }

    void clear();
    void reset();
    void saw(float* output, float* sync, int samples);
    void square(float* output, float* sync, int samples);
    void saw2(float* output, float* sync, int samples);
    void square2(float* output, float* sync, int samples);
    void process(float* output, float* sync, int samples);

};

/**
 * Noise
 */
class Noise : public Oscillator {

    enum {WHITE, PINK, LP, BP};

    StateVariableFilter2 filter;

    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;

  public:
    void setSamplerate(float r) {
        Oscillator::setSamplerate(r);
        filter.setSamplerate(r);
    }

    void process(float* output, float* sync, int samples);
};

}

#endif
