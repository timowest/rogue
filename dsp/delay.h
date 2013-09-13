/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_DELAY_H
#define DSP_DELAY_H

#include "types.h"

namespace dsp {


/**
* allpass interpolating delay line class.
*
* This class implements a fractional-length digital delay-line using
* a first-order allpass filter.
*/
class DelayA {
   static const uint length = 4096;

  public:
    DelayA();
    void setDelay(float d);
    void clear();
    float nextOut();
    float process(float in);

  private:
    float buffer_[length];
    uint inPoint_, outPoint_;
    float delay_, alpha_, coeff_, last_ = 0.0;
    float apInput_ = 0.0, nextOutput_ = 0.0;
    bool doNextOut_ = true;
};

}

#endif
