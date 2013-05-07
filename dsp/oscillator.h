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

    void setParams(float a1f, float a1t, float a0f, float a0t, float wf, float wt) {
      this->a1f = a1f;
      this->a1t = a1t;
      this->a0f = a0f;
      this->a0t = a0t;
      this->wf = wf;
      this->wt = wt;
    }

    void setParams(float a1, float a0, float w) {
        setParams(a1, a1, a0, a0, w, w);
    }

    void setType(int t) { type = t; }
    void setSamplerate(float r) { sample_rate = r; }
    void setBandlimit(bool b) { bandlimit = b; }
    void process(float* output, int samples);

  protected:
    float sin2(float p);
    void sin2(float* output, int samples);
    void hardsync(float* output, int samples);
    void softsync(float* output, int samples);
    void pulse(float* output, int samples);
    void slope(float* output, int samples);
    void jp8000_tri(float* output, int samples);
    void jp8000_supersaw(float* output, int samples);
    void waveslices(float* output, int samples);
    void sinusoids(float* output, int samples);
    void noise (float* output, int samples);

  private:
    bool bandlimit = true;
    float phase, freq;
    //float width = 0.5f, a1 = 1.0f, a0 = 0.0f;
    float a1f = 1.0f, a1t = 1.0f, a0f = 0, a0t = 0;
    float wf = 0.5, wt = 0.5;
    float sample_rate;
    int type;
};

}

#endif
