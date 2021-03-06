/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_FILTER_H
#define DSP_FILTER_H

#include "delay.h"

namespace dsp {

/**
 * abstract filter class.
 */
class Filter {
  public:
    virtual void clear() = 0;
    virtual void process(float* input, float* output, int samples) = 0;
};

/**
 * DC offset correction filter
 */
class DCBlocker : Filter {
  public:
    void setSamplerate(float r);
    void clear();
    void process(float* input, float* output, int samples);

  private:
    float R, x1 = 0.0f, y1 = 0.0f;
};

/**
 * one-pole filter class.
 *
 * This class implements a one-pole digital filter. A method is
 * provided for setting the pole position along the real axis of the
 * z-plane while maintaining a constant peak filter gain.
 */
class OnePole : Filter {
  public:
    void clear();
    float getLast();
    void setCoefficients(float b0, float a1);
    void setPole(double p);
    void setLowpass(double fc);
    void setHighpass(double fc);
    float process(float input);
    void process(float* input, float* output, int samples);

  private:
    float b0_, a1_, last_;
};

/**
 * one-zero filter class.
 *
 * This class implements a one-zero digital filter. A method is
 * provided for setting the zero position along the real axis of the
 * z-plane while maintaining a constant filter gain.
 */
class OneZero : Filter {
  public:
    void clear();
    void setCoefficients(float b0, float b1);
    void setZero(float z);
    float process(float input);
    void process(float* input, float* output, int samples);

  protected:
    float b0_, b1_, last_, prevIn_;
};

/**
 * one-pole, one-zero filter class.
 *
 * This class implements a one-pole, one-zero digital filter. A
 * method is provided for creating an allpass filter with a given
 * coefficient. Another method is provided to create a DC blocking
 * filter.
 */

class PoleZero : Filter {
  public:
    void clear();
    void setCoefficients(float b0, float b1, float a1);
    void setAllpass(float coefficient);
    void setBlockZero(float thePole = 0.99);
    float process(float input);
    void process(float* input, float* output, int samples);

  private:
    float a0_, a1_, b0_, b1_, last_, prevIn_;
};

/**
 * two-pole filter class.
 *
 * This class implements a two-pole digital filter. A method is
 * provided for creating a resonance in the frequency response while
 * maintaining a nearly constant filter gain.
 */
class TwoPole : Filter {
  public:
    void clear();
    void setCoefficients(float b0, float a1, float a2);
    float process(float input);
    void process(float* input, float* output, int samples);

  private:
    float b0_, a1_, a2_, last_, last__;
};

/**
 * two-zero filter class.
 *
 * This class implements a two-zero digital filter. A method is
 * provided for creating a "notch" in the frequency response while
 * maintaining a constant filter gain.
 */
class TwoZero : Filter {
  public:
    void clear();
    void setCoefficients(float b0, float b1, float b2);
    float process(float input);
    void process(float* input, float* output, int samples);

  private:
    float b0_, b1_, b2_, last_, prevIn_, prevIn__;
};

/**
 * biquad (two-pole, two-zero) filter class.
 *
 * This class implements a two-pole, two-zero digital filter.
 * Methods are provided for creating a resonance or notch in the
 * frequency response while maintaining a constant filter gain.
 */
class BiQuad : Filter {
  public:
    void clear();
    void setCoefficients(float b0, float b1, float b2, float a1, float a2);
    float process(float input);
    void process(float* input, float* output, int samples);

  private:
    float b0_, b1_, b2_, a1_, a2_, last_, prevIn_, prevIn__, last__;
};

class BiQuadDF2 : Filter {
  public:
    void clear();
    void setCoefficients(float b0, float b1, float b2, float a1, float a2);
    void setLowpass(float fc, float res);
    void setHighpass(float fc, float res);
    float process(float input);
    void process(float* input, float* output, int samples);

  private:
    float b0_, b1_, b2_, a1_, a2_, z1_, z2_;
};

/**
 * AmSynth filter
 * Copyright (c) 2001-2012 Nick Dowell
 */
class AmSynthFilter : Filter {

    enum {LP24, HP24, BP24, LP12, HP12, BP12};

  public:
    void clear();
    void setType(int t) { type_ = t;}
    void setSamplerate(float r) { sample_rate_ = r; }
    void setCoefficients(float f, float r);
    void process(float* input, float* output, int samples);

  private:
    double d1, d2, d3, d4;
    float freq_, res_, sample_rate_;
    int type_ = 0;
};

/**
 * Moog 24 dB/oct resonant lowpass VCF
 * References: CSound source code, Stilson/Smith CCRMA paper.
 * Modified by paul.kellett@maxim.abel.co.uk July 2000
 */
class MoogFilter : Filter {

  public:
    void clear();
    void setType(int t) { type = t; }
    void setSamplerate(float r) { sample_rate = r; }
    void setCoefficients(float f, float r);
    void process(float* input, float* output, int samples);

  private:
    float f, pc, q;
    float bf0, bf1, bf2, bf3, bf4;
    float t1, t2;

    int type = 0;
    float sample_rate;
};

/**
 * Andrew Simper's State Variable Filter
 *
 * based on http://www.musicdsp.org/showone.php?id=92
*/
class StateVariableFilter : Filter {

    enum {LP, HP, BP, NOTCH};

  public:
    void clear();
    void setType(int type);
    void setSamplerate(float rate) { sample_rate = rate; }
    void setCoefficients(float fc, float res);
    void setDistortion(float d) { drive = d; }
    void process(float* input, float* output, int samples);

  private:
    float* out;
    float notch, low, high, band;
    float freq, damp;
    float drive; // internal distortion
    float sample_rate;
};

/**
 * Andrew Simper's State Variable Filter (improved version)
 *
 * based on Linear Trapezoidal Integrated State Variable Filter With Low Noise Optimisation
 * (2011)
*/
class StateVariableFilter2 : Filter {

    enum {LP, HP, BP, NOTCH};

  public:
    void clear();
    void setType(int t) { type = t; }
    void setSamplerate(float rate) { sample_rate = rate; }
    void setCoefficients(float fc, float res);
    void process(float* input, float* output, int samples);

  private:
    float sample_rate;
    int type = 0;

    float v0z, v1, v2;
    float k, g1, g2, g3, g4;

};

/**
 * Comb filter
 */
class CombFilter : Filter {

  public:
    void clear();
    void setSamplerate(float rate) { sample_rate = rate; }
    void setCoefficients(float fc, float amount);
    void process(float* input, float* output, int samples);

  private:
    float sample_rate, fc, amount;
    DelayA delay;

};

}

#endif
