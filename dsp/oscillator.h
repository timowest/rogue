/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_OSCILLATOR_H
#define DSP_OSCILLATOR_H

namespace dsp {

/**
 * abstract oscillator class
 */
class Oscillator {
  protected:
    float freq = 440.0f, sample_rate, phase = 0.0f;
    float tone = 0.5f, wf = 0.5f, wt = 0.5f;
    int type = 0;

  public:
    void setType(int t) { type = t; }
    virtual void setSamplerate(float r) { sample_rate = r; }
    void setFreq(float f) { freq = f; }

    void setParams(float _t, float _wf, float _wt) {
        tone = _t;
        wf = _wf;
        wt = _wt;
    }

    void clear() {
        phase = 0.0f;
        freq = 440.0f;
        type = 0;

        tone = wf = wt = 0.5;
    }

    void reset() {
        phase = 0;
    }

    virtual void process(float* output, int samples) = 0;
};

/**
 * Phase Distortion
 *
 * Implements the waveforms of Casio CZ synths
 */
class PD : public Oscillator {

    enum {SAW, SQUARE, PULSE, DOUBLE_SINE, SAW_PULSE, RES1, RES2, RES3, HALF_SINE};

  public:
    void saw(float* output, int samples);
    void square(float* output, int samples);
    void pulse(float* output, int samples);
    void double_sine(float* output, int samples);
    void saw_pulse(float* output, int samples);
    void res1(float* output, int samples);
    void res2(float* output, int samples);
    void res3(float* output, int samples);
    void half_sine(float* output, int samples);
    void process(float* output, int samples);

};

// TODO sync saw, sync square

/**
 * Electronic
 */
class EL : public Oscillator {

    enum {SAW, DOUBLE_SAW, TRI, TRI2, TRI3, PULSE, PULSE_SAW, SLOPE, ALPHA1, ALPHA2};

  public:
    void saw(float* output, int samples);
    void double_saw(float* output, int samples);
    void tri(float* output, int samples);
    void tri2(float* output, int samples);
    void tri3(float* output, int samples);
    void pulse(float* output, int samples);
    void pulse_saw(float* output, int samples);
    void slope(float* output, int samples);
    void alpha1(float* output, int samples);
    void alpha2(float* output, int samples);
    void process(float* output, int samples);
};


// TODO JP8000 tri mod, supersaw

/**
 * Virtual Analog
 */
class VA : public Oscillator {

    enum {SAW, TRI_SAW, PULSE};

    float prev = 0.0f;
    EL el;

  public:
    void setSamplerate(float r) {
        Oscillator::setSamplerate(r);
        el.setSamplerate(r);
    }

    void highpass(float* output, int samples);
    void saw(float* output, int samples);
    void tri_saw(float* output, int samples);
    void pulse(float* output, int samples);
    void process(float* output, int samples);

};

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

  public:
    void process(float* output, int samples);
};

}

#endif
