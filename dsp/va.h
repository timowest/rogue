/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_VA_H
#define DSP_VA_H

namespace dsp {

/**
 * Various Minimoog emulations
 *
 * - Sawooth based on "Discrete-Time Modelling of the Moog Sawtooth Oscillator Waveform"
 */
class VA {

    enum {TRI, SAW, SAW_TRI, PULSE};

  public:
    void clear();
    void reset();
    void setFreq(float f) { freq = f; }
    void setWidth(float w) { width = w; }
    void setType(int t) { type = t; }
    void setSamplerate(float r) { sample_rate = r; }
    void setBandlimit(bool b) { bandlimit = b; }
    void process(float* output, int samples);

  private:
    bool bandlimit = true;
    float phase, freq;
    float width = 0.5f;
    float sample_rate;
    int type;
};

}

#endif
