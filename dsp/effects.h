/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_EFFECTS_H
#define DSP_EFFECTS_H

#include "types.h"
#include "delay.h"
#include "filter.h"

namespace dsp {

class Effect {
  public:
    virtual void clear() = 0;
    virtual void process(float* left, float* right, int samples) = 0;
    virtual void setSamplerate(float r) = 0;
};

// Chorus
// TODO - lp after delay or in feedback

class ChorusEffect : Effect {
    DelayL delay_l, delay_r;
    float lfo_phase = 0, lfo_inc;
    float delay, amount, rate, depth, feedback;
    float last_l = 0, last_r = 0;
    float sample_rate;

  public:
    ChorusEffect() {}
    void clear();
    void process(float* left, float* right, int samples);
    void setCoefficients(float d, float a, float r, float de, float fb);
    void setSamplerate(float r);
};

// Phaser
// TODO inline allpass delays

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
    float lfo_phase = 0, lfo_inc;
    float delay, amount, rate, depth, feedback;
    float last_l = 0, last_r = 0;
    float sample_rate;

  public:
    PhaserEffect() {}
    void clear();
    void process(float* left, float* right, int samples);
    void setCoefficients(float del, float a, float r, float d, float fb);
    void setSamplerate(float r);
};

// Delay

class DelayEffect : Effect {
    Delay delay_l, delay_r;
    OnePole filter_1l, filter_2l, filter_1r, filter_2r;
    float depth, feedback;
    float lowcut, highcut;
    float last_l = 0, last_r = 0;
    float sample_rate;

  public:
    DelayEffect() {}
    void clear();
    void process(float* left, float* right, int samples);
    void setCoefficients(float b, float di_l, float di_r, float de, float fb, float lc, float hc);
    void setSamplerate(float r);
};

// Reverb

/*class ReverbEffect : Effect {

};*/

}

#endif

