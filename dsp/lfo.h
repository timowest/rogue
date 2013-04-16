/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_LFO_H
#define DSP_LFO_H

namespace dsp {

class LFO {

    enum {SIN, TRI, SAW, SQUARE, SH};

  public:
    void clear ();
    void reset ();
    void setType(int t) { type = t; }
    void setFreq(float f) { freq = f; }
    void setSamplerate(float r) { sample_rate = r; }
    void setSymmetry(float s) { symmetry = s; }
    float getValue(float p);
    float tick(int samples);
    float tick();

  private:
    int type = 0;
    float phase = 0.0, freq, symmetry = 0.5;
    float prev_phase, value;
    float sample_rate;
};

}

#endif
