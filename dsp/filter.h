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
    void setCoefficients(float b0, float a1);
    void setPole(double p);
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

/**
 * Tim Stilson's MoogVCF filter using 'compromise' poles at z = -0.3
 *
 * Several improvements are built in, such as corrections for cutoff
 * and resonance parameters, removal of the necessity of the
 * separation table, audio rate update of cutoff and resonance
 * and a smoothly saturating tanh() function, clamping output and
 * creating inherent nonlinearities.
 *
 * Much credit is owed to Antti Huovilainen's 2004 and 2006 papers
 * on the subject.
 *
 * originally by Aaron Krajeski 2012
 * based on http://song-swap.com/MUMT618/aaron/Presentation/
 */
class MoogFilter : Filter {

    enum {LP24, LP18, LP12, LP6, HP24, BP12, BP18, NOTCH};

  public:
    void clear();
    void setType(int t) { type_ = t; }
    void setSamplerate(float r) { sample_rate_ = r; }
    void setCoefficients(float freq, float res);
    void process(float* input, float* output, int samples);

  private:
    float dlout_[5], dlin_[5];
    float drive_ = 1.0f, wc_, g_, gres_, gcomp_ = 0.5f;
    float sample_rate_;
    int type_;
};

/**
 * This model is based on a reference implementation of an algorithm developed by
 * Stefano D'Angelo and Vesa Valimaki, presented in a paper published at ICASSP in 2013.
 * This improved model is based on a circuit analysis and compared against a reference
 * Ngspice simulation. In the paper, it is noted that this particular model is
 * more accurate in preserving the self-oscillating nature of the real filter.
 * References: "An Improved Virtual Analog Model of the Moog Ladder Filter"
 * Original Implementation: D'Angelo, Valimaki
 */
class MoogFilter2 : Filter {

  public:
    void clear();
    void setType(int t) { type_ = t; }
    void setSamplerate(float r) { sample_rate_ = r; }
    void setCoefficients(float freq, float res);
    void process(float* input, float* output, int samples);

  private:
    float sample_rate_;
    int type_;

    float V1prev, V2prev, V3prev, V4prev;
    float tV1prev, tV2prev, tV3prev, tV4prev;
    float dV1prev, dV2prev, dV3prev, dV4prev;

    float _x;
    float _g;

    float _cutoff;
    float _resonance;
    float _drive;
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
