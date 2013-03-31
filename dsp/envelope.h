/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_ENVELOPE_H
#define DSP_ENVELOPE_H

namespace dsp {

/**
 * ADSR envelope class.
 *
 * This class implements a traditional ADSR (Attack, Decay, Sustain,
 * Release) envelope. It responds to simple keyOn and keyOff
 * messages, keeping track of its state. The \e state = ADSR::IDLE
 * before being triggered and after the envelope value reaches 0.0 in
 * the ADSR::RELEASE state. All rate, target and level settings must
 * be non-negative. All time settings must be positive.
 */
class ADSR {

    enum {A, D, S, R, IDLE };

  public:
    void on();
    void off();
    void setADSR(float _a, float _d, float _s, float _r);
    float lastOut() { return last; }
    int state() { return state_; }
    float tick(int samples);
    float tick();

  private:
    float attackTarget = 1.0; // TODO : make modifiable
    float attackRate, decayRate, releaseSamples, releaseRate = 0.0;
    float sustain = 0.5;
    float last = 0.0;
    int state_ = IDLE;
};

}

#endif
