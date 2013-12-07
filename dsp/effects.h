/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_EFFECTS_H
#define DSP_EFFECTS_H

#include <math.h>
#include "types.h"
#include "delay.h"
#include "filter.h"
#include "lfo.h"

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

class AllpassDelay {
    float a1 = 0, zm1 = 0;

  public:
    void clear();
    float process(float in);
    float process(float a1, float in);
    void setDelay(float d);
};

class PhaserEffect : Effect {
    AllpassDelay filters_l[8];
    AllpassDelay filters_r[8];
    float lfo_phase = 0, lfo_inc;
    float min_d, max_d, delta_d, rate, depth, feedback;
    float last_l = 0, last_r = 0;
    float sample_rate;

  public:
    PhaserEffect() {}
    void clear();
    void process(float* left, float* right, int samples);
    void setCoefficients(float min_fr, float max_fr, float r, float d, float fb);
    void setSamplerate(float r);
};

// Delay

class DelayEffect : Effect {
    Delay delay_l, delay_r;
    OnePole filter_1l, filter_2l, filter_1r, filter_2r;
    float depth, feedback, direct, pingpong;
    float lowcut, highcut;
    float last_l = 0, last_r = 0;
    float sample_rate;

  public:
    DelayEffect() {}
    void clear();
    void process(float* left, float* right, int samples);
    void setCoefficients(float b, float di_l, float di_r, float de, float fb, float pp, float lc, float hc);
    void setSamplerate(float r);
};

// Reverb (FDN based)

// Csound orchestra version coded by Sean Costello, October 1999
// C implementation (C) 2005 Istvan Varga

class ReverbEffect : Effect {
    static const uint ER_DELAYS = 12;

    // early reflections
    DelayL erDelays[2]; // stereo delay

    // late reverb
    AllpassDelay adelays[8];
    DelayL delays[8];
    OnePole filters[8];
    LFO lfos[8];
    float sample_rate, samples_per_meter;
    float gain = 0.9, pitchmod = 1.0, tone = 5000.0, depth = 0.0;

  public:
    ReverbEffect();
    void clear();
    void setErCoefficients(float pre_delay, float spread);
    void setCoefficients(float gain, float pitchmod, float tone, float depth);
    void process(float* left, float* right, int samples);
    void setSamplerate(float r);
};

}

#endif

