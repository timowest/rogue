/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_OSCILLATOR_H
#define DSP_OSCILLATOR_H

#include "filter.h"

namespace dsp {

/**
 * abstract oscillator class
 */
class Oscillator {
  protected:
    float freq = 440.0f, sample_rate, phase = 0.0f;
    float start = 0.0f, wf = 0.5f, wt = 0.5f;
    int type = 0;

    // audio modulation
    float* input;
    float pm = 0.0f;
    bool sync;

  public:
    void setType(int t) { type = t; }
    virtual void setSamplerate(float r) { sample_rate = r; }
    void setFreq(float f) { freq = f; }

    /** phase modulation */
    float pmod(float phase, int i) {
        float phase2 = phase + pm * input[i];
        if (phase2 < 0.0f) {
            phase2 += 1.0f;
        } else if (phase2 > 1.0f) {
            phase2 -= 1.0f;
        }
        return phase2;
    }

    void setModulation(float* _input, float _pm, bool _sync) {
        input = _input;
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

    void clear() {
        phase = start;
        freq = 440.0f;
        type = 0;

        wf = wt = 0.5;
    }

    void reset() {
        phase = start;
    }

    virtual void process(float* output, int samples) = 0;
};

/** Virtual
 *  Analog - Minimoog waveform models
 *  Phase Distortion - Casio CZ sounds
 *  Electronic - various
 *  FM - OPL FM sounds
 */

class Virtual : public Oscillator {

    // 34
    enum {VA_SAW, VA_TRI_SAW, VA_PULSE,
          PD_SAW, PD_SQUARE, PD_PULSE, PD_DOUBLE_SINE, PD_SAW_PULSE, PD_RES1, PD_RES2, PD_RES3, PD_HALF_SINE,
          EL_SAW, EL_DOUBLE_SAW, EL_TRI, EL_TRI2, EL_TRI3, EL_PULSE, EL_PULSE_SAW, EL_SLOPE, EL_ALPHA1, EL_ALPHA2,
          EL_BETA1, EL_BETA2, EL_PULSE_TRI, EL_EXP,
          FM1, FM2, FM3, FM4, FM5, FM6, FM7, FM8
    };

    float prev = 0.0f;

  public:
    // VA
    void va_highpass(float* output, int samples);
    void va_saw(float* output, int samples);
    void va_tri_saw(float* output, int samples);
    void va_pulse(float* output, int samples);

    // PD
    void pd_saw(float* output, int samples);
    void pd_square(float* output, int samples);
    void pd_pulse(float* output, int samples);
    void pd_double_sine(float* output, int samples);
    void pd_saw_pulse(float* output, int samples);
    void pd_res1(float* output, int samples);
    void pd_res2(float* output, int samples);
    void pd_res3(float* output, int samples);
    void pd_half_sine(float* output, int samples);

    // EL
    void el_saw(float* output, int samples);
    void el_double_saw(float* output, int samples);
    void el_tri(float* output, int samples);
    void el_tri2(float* output, int samples);
    void el_tri3(float* output, int samples);
    void el_pulse(float* output, int samples);
    void el_pulse2(float* output, int samples);
    void el_pulse_saw(float* output, int samples);
    void el_slope(float* output, int samples);
    void el_alpha1(float* output, int samples);
    void el_alpha2(float* output, int samples);
    void el_beta1(float* output, int samples);
    void el_beta2(float* output, int samples);
    void el_pulse_tri(float* output, int samples);
    void el_exp(float* output, int samples);

    // FM
    void fm1(float* output, int samples);
    void fm2(float* output, int samples);
    void fm3(float* output, int samples);
    void fm4(float* output, int samples);
    void fm5(float* output, int samples);
    void fm6(float* output, int samples);
    void fm7(float* output, int samples);
    void fm8(float* output, int samples);

    void process(float* output, int samples);

};

// TODO JP8000 tri mod, supersaw

/**
 * Additive Synthesis
 */
class AS : public Oscillator {

    enum {SAW, SQUARE, IMPULSE};

  public:
    void saw(float* output, int samples);
    void square(float* output, int samples);
    void impulse(float* output, int samples);

    void process(float* output, int samples);
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

    void process(float* output, int samples);
};

}

#endif
