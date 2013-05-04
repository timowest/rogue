/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_ENVELOPE_H
#define DSP_ENVELOPE_H

namespace dsp {

/**
 * AHDSR envelope class
 */
class AHDSR {

    enum {LEAD3, LEAD2, LEAD1, LINEAR, LAG1, LAG2, LAG3};

    enum {A, H, D, S, R, IDLE };

  public:
    void on();
    void off();
    void setAHDSR(float _a, float _h, float _d, float _s, float _r);
    void setCurve(float t) { a = 1.0f - 1.0f/t; }
    float lastOut() { return last; }
    int state() { return state_; }
    float tick(int samples);
    float tick();

  private:
    float envCurve(float x);
    float innerTick();
    float attackTarget = 1.0;
    float attackRate, holdSamples, decayRate, releaseRate = 0.0;
    float sustain = 0.5;
    float last = 0.0;
    float a = -1.0f;
    float scale, offset;
    int state_ = IDLE;
    int counter;
};

}

#endif
