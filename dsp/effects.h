/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_EFFECTS_H
#define DSP_EFFECTS_H

#include "types.h"
#include "delay.h"
#include "lfo.h"

namespace dsp {

class Effect {
  public:
    virtual void clear() = 0;
    virtual void process(float* left, float* right, int samples) = 0;
    virtual void setSamplerate(float r) = 0;
};

// Chorus

class ChorusEffect : Effect {
    DelayA delay_l, delay_r;
    LFO lfo_l, lfo_r;
    float delay, amount, rate, feedforward, feedback;
    float last_l, last_r;
    float sample_rate;

  public:
    ChorusEffect() {
        lfo_r.setStart(0.5);
    }
    void clear();
    void process(float* left, float* right, int samples);
    void setCoefficients(float d, float a, float r, float ff, float fb);
    void setSamplerate(float r);
};

// Phaser

class AllpassDelay {
    float a1 = 0, zm1 = 0;

  public:
    void clear();
    float process(float in);
    void setDelay(float d);
};

class PhaserEffect : Effect {
    AllpassDelay filters_l[8];
    AllpassDelay filters_r[8];
    LFO lfo_l, lfo_r;
    float delay, amount, rate, depth, feedback;
    float last_l, last_r;
    float sample_rate;

  public:
    PhaserEffect() {
        lfo_r.setStart(0.5);
    }
    void clear();
    void process(float* left, float* right, int samples);
    void setCoefficients(float del, float a, float r, float d, float fb);
    void setSamplerate(float r);
};

// Delay

class DelayEffect : Effect {
    Delay delay_l, delay_r;
    float delay,  amount, feedback;
    float last_l, last_r;
    float sample_rate;

  public:
    DelayEffect() {}
    void clear();
    void process(float* left, float* right, int samples);
    void setCoefficients(float b, float di, float a, float fb);
    void setSamplerate(float r);
};

// Reverb

/*class ReverbEffect : Effect {

};*/

}

#endif

