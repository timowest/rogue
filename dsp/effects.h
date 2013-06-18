/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * based on synthv1 (http://synthv1.sourceforge.net/)
 * Copyright 2013 Rui Nuno Capela
 */

#ifndef DSP_EFFECTS_H
#define DSP_EFFECTS_H

namespace dsp {

class Effect {
  protected:
    float sample_rate;

  public:
    void setSamplerate(float r) { sample_rate = r; }
    virtual void reset() = 0;
    virtual void process(float* l, float* r, int samples) = 0;
};

// Flanger

class Flanger {
    static const uint32_t MAX_SIZE = 4096;  //= (1 << 12);
    static const uint32_t MAX_MASK = MAX_SIZE - 1;

    float buffer[MAX_SIZE];
    int frames;

  public:
    void reset();
    float output(float in, float delay, float feedb);
};

// Chorus

class Chorus : Effect {
    float delay, feedback, rate, mod, wet;

    Flanger flanger1, flanger2;
    float lfo_phase = 0.0f;

  public:
    void reset();
    void process(float* l, float* r, int samples);
};

// Phaser

class APDelay {
    float a = 0.0, m = 0.0;

  public:
    void set (float a_) { a = a_;}

    void reset() { m = 0.0f; }

    float process (float x) {
        float y = -a * x + m;
        m = a * y + x;
        return y;
    }
};

class Phaser : Effect {
    float lfo_rate, depth, feedback, wet;

    APDelay ap_l[6], ap_r[6];
    float lfo_phase = 0.0, out_l = 0.0, out_r;

  public:
    void reset();
    void process(float* l, float* r, int samples);
};


// Delay

class Delay : Effect {

  public:
    void process(float* l, float* r, int samples);
};

// Reverb

class Reverb : Effect {

  public:
    void process(float* l, float* r, int samples);
};

}

#endif
