/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_OSCILLATOR_H
#define DSP_OSCILLATOR_H

namespace dsp {

/**
 * Phaseshaping Oscillator
 *
 * PhaseShaping uses cascade phase shaping functions to create classic
 * and novel waveforms.
 * Bandlimiting is used where feasible.
 *
 * based on Phaseshaping Oscillator Algorithms for Musical Sound Synthesis
 */
class PhaseShaping {

    enum {SIN, HARD, SOFT, PULSE, SLOPE, TRI, SUPERSAW, SLICES, SINUSOIDS, NOISE};

  public:
    void clear ();
    void reset ();
    void setFreq(float f) { freq = f; }
    void setWidth(float w);
    void setParams(float a1, float a0);
    void setType(int t) { type = t; }
    void setSamplerate(float r) { sample_rate = r; }
    void process(float* output, int samples);

  protected:
    float sin2 (float in);
    float hardsync (float in);
    float softsync (float in);
    float pulse (float in);
    float slope (float in);
    float jp8000_tri (float in);
    float jp8000_supersaw (float in);
    float waveslices (float in);
    float sinusoids (float in);
    float noise ();

  private:
    float phase, freq;
    float width = 0.5f, a1 = 1.0f, a0 = 0.0f;
    float sample_rate;
    int type;
};

}

#endif
