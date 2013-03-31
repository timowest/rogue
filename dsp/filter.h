/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_FILTER_H
#define DSP_FILTER_H

namespace dsp {

/**
 * abstract filter class.
 */
class Filter {
  public:
    virtual void clear();
    virtual void process(float* input, float* output, int samples) = 0;
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
    void process(float* input, float* output, int samples);

  private:
    float b0_, b1_, b2_, a1_, a2_, last_, prevIn_, prevIn__, last__;
};

/**
 * Tim Stilson's MoogVCF filter using 'compromise' poles at z = -0.3
 *
 * Several improments are built in, such as corrections for cutoff
 * and resonance parameters, removal of the necessity of the
 * separation table, audio rate update of cutoff and resonance
 * and a smoothly saturating tanh() function, clamping output and
 * creating inherent nonlinearities.
 *
 * Much credit is owed to Antti Huovilainen's 2004 and 2006 papers
 * on the subject.
 *
 * originally by Aaron Krajeski 2012 *
 * based on http://song-swap.com/MUMT618/aaron/Presentation/
 */
class MoogFilter : Filter {

    enum {LP24, LP18, LP12, LP6, HP24, BP12, BP18, NOTCH };

  public:
    void clear();
    void setType(int t) { type_ = t; }
    void setSamplerate(float r) { sample_rate_ = r; }
    void setCoefficients(float freq, float res);
    void process(float* input, float* output, int samples);

  protected:
    void compute(float in);

  private:
    float dlout_[5], dlin_[5];
    float drive_, wc_, g_, gres_, gcomp_;
    float sample_rate_;
    int type_;
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
    void process(float* input, float* output, int samples);

  private:
    float* out;
    float notch, low, high, band;
    float freq, damp;
    float drive; // internal distortion
    float sample_rate;
};

// TODO CombFilter

}

#endif
