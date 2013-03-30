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

    void process (float* output, int samples);

  private:
    float phase, freq, a1, a0;
    int sampleRate, type;
};

}

#endif
