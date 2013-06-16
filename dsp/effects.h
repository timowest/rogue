/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_EFFECTS_H
#define DSP_EFFECTS_H

namespace dsp {

class Effect {
  public:
    virtual void process(float* l, float* r, int samples) = 0;
};

// Chorus

class Chorus : Effect {

  public:
    void process(float* l, float* r, int samples) {
        // TODO
    }
};

// Phaser

class Phaser : Effect {

  public:
    void process(float* l, float* r, int samples) {
      // TODO
    }
};

// Delay

class Delay : Effect {

  public:
    void process(float* l, float* r, int samples) {
      // TODO
    }
};

// Reverb

class Reverb : Effect {

  public:
    void process(float* l, float* r, int samples) {
      // TODO
    }
};

}

#endif
