/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * This header is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License,
 * or (at your option) any later version.
 *
 * This header is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA.
 */

#include <algorithm>
#include <math.h>

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
    void clear() {
        last_ = 0.0;
    }

    void setCoefficients(float b0, float a1) {
        b0_ = b0;
        a1_ = a1;
    }

    void setPole(double p) {
        b0_ = (p > 0.0) ? (1.0 - p) : (1.0 + p);
        a1_ = -p;
    }

    void process(float* input, float* output, int samples) {
        for (int i = 0; i < samples; i++) {
            last_ = b0_ * input[i] - a1_ * last_;
            output[i] = last_;
        }
    }

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
    void clear() {
        last_ = prevIn_ = 0.0;
    }

    void setCoefficients(float b0, float b1) {
        b0_ = b0;
        b1_ = b1;
    }

    void setZero(float z) {
        b0_ = (z > 0.0) ? (1.0 + z) : (1.0 - z);
        b1_ = -z * b0_;
    }

    void process(float* input, float* output, int samples) {
        for (int i = 0; i < samples; i++) {
            last_ = b0_ * input[i] + b1_ * prevIn_;
            prevIn_ = input[i];
            output[i] = last_;
        }
    }

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
    void clear() {
        last_ = prevIn_ = 0.0;
    }

    void setCoefficients(float b0, float b1, float a1) {
        b0_ = b0;
        b1_ = b1;
        a1_ = a1;
    }

    void setAllpass(float coefficient) {
        b0_ = coefficient;
        b1_ = 1.0;
        a0_ = 1.0; // just in case
        a1_ = coefficient;
    }

    void setBlockZero(float thePole = 0.99) {
        b0_ = 1.0;
        b1_ = -1.0;
        a0_ = 1.0; // just in case
        a1_ = -thePole;
    }

    void process(float* input, float* output, int samples) {
        for (int i = 0; i < samples; i++) {
            last_ = b0_ * input[i] + b1_ * prevIn_ - a1_ * last_;
            prevIn_ = input[i];
            output[i] = last_;
        }
    }

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
    void clear() {
        last_ = last__ = 0.0;
    }

    void setCoefficients(float b0, float a1, float a2) {
        b0_ = b0;
        a1_ = a1;
        a2_ = a2;
    }

    void process(float* input, float* output, int samples) {
        for (int i = 0; i < samples; i++) {
            float temp = last_;
            last_ = b0_ * input[i] - a1_ * last_ - a2_ * last__;
            last__ = temp;
            output[i] = last_;
        }
    }

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
    void clear() {
        last_ = prevIn_ = prevIn__ = 0.0;
    }

    void setCoefficients(float b0, float b1, float b2) {
        b0_ = b0;
        b1_ = b1;
        b2_ = b2;
    }

    void process(float* input, float* output, int samples) {
        for (int i = 0; i < samples; i++) {
            last_ = b0_ * input[i] + b1_ * prevIn_ + b2_ * prevIn__;
            prevIn__ = prevIn_;
            prevIn_ = input[i];
            output[i] = last_;
        }
    }

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
    void clear() {
        last_ = last__ = prevIn_ = prevIn__ = 0.0;
    }

    void setCoefficients(float b0, float b1, float b2, float a1, float a2) {
        b0_ = b0;
        b1_ = b1;
        b2_ = b2;
        a1_ = a1;
        a2_ = a2;
    }

    void process(float* input, float* output, int samples) {
        for (int i = 0; i < samples; i++) {
            float temp = last_;
            last_ = b0_ * input[i] + b1_ * prevIn_ + b2_ * prevIn__;
            last_ -= a1_ * temp + a2_ * last__;
            prevIn__ = prevIn_;
            prevIn_ = input[i];
            last__ = temp;
            output[i] = last_;
        }
    }

  private:
    float b0_, b1_, b2_, a1_, a2_, last_, prevIn_, prevIn__, last__;
};

#define MOOG_LOOP(x) \
    for (int i = 0; i < samples; i++) { \
        compute(input[i]); \
        output[i] = x; \
    } \
    break

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

    enum {LP24, LP18, LP12, LP6, HP24, BP12, BP18, NOTCH};

  public:
    void clear() {
        drive_ = 1.0;
        type_ = 0;
        gcomp_ = 0.5;
        dlout_[0] = dlout_[1] = dlout_[2] = dlout_[3] = dlout_[4] = 0.0;
        dlin_[0] = dlin_[1] = dlin_[2] = dlin_[3] = dlin_[4] = 0.0;
    }

    void setType(int t) {
        type_ = t;
    }

    void setCoefficients(float freq, float res) {
        wc_ = 2 * M_PI * freq / sampleRate_;
        g_ = 0.9892 * wc_ - 0.4342 * pow(wc_, 2) + 0.1381 * pow(wc_, 3) - 0.0202 * pow(wc_, 4);
        gres_ = res * (1.0029 + 0.0526 * wc_ - 0.926 * pow(wc_, 2) + 0.0218 * pow(wc_, 3));
    }

    void compute(float in) {
        // first input (with saturation and feedback)
        dlout_[0] = (float) tanh(drive_ * (in - 4 * gres_ * (dlout_[4] - gcomp_ * in)));

        // four filter blocks
        for (int i = 0; i < 4; i++) {
            dlout_[i+1] = g_ * (0.3/1.3 * dlout_[i] + 1/1.3 * dlin_[i] - dlout_[i + 1]) + dlout_[i + 1];
            dlin_[i] = dlout_[i];
        }
    }

    void process(float* input, float* output, int samples) {
        switch(type_) {
        case LP24: // 24dB LP
            MOOG_LOOP(dlout_[4]);
        case LP18: // 18dB LP
            MOOG_LOOP(dlout_[3]);
        case LP12: // 12dB LP
            MOOG_LOOP(dlout_[2]);
        case LP6:  // 6db LP
            MOOG_LOOP(dlout_[1]);
        case HP24: // 24dB HP
            MOOG_LOOP(dlout_[0] - dlout_[4]);
        case BP12: // 12db BP
            MOOG_LOOP(dlout_[4] - dlout_[2]);
        case BP18: // 18/6dB BP
            MOOG_LOOP(dlout_[3] - dlout_[4]);
        case NOTCH:// NOTCH
            MOOG_LOOP((dlout_[3] - dlout_[4]) + 2/3 * dlout_[0]);
        }
    }

  private:
    float dlout_[5], dlin_[5];
    float drive_, wc_, g_, gres_, gcomp_;
    int sampleRate_, type_;

};

/**
 * Andrew Simper's State Variable Filter
 *
 * based on http://www.musicdsp.org/showone.php?id=92
 */
class StateVariableFilter : Filter {

    enum {LP, HP, BP, NOTCH};

  public:
    void clear() {
        drive = 0.0;
        notch = low = high = band = 0.0;
        out = &low;
    }

    void setType(int type) {
        switch(type) {
        case LP:
            out = &low; break;
        case HP:
            out = &high; break;
        case BP:
            out = &band; break;
        case NOTCH:
            out = &notch; break;
        }
    }

    void setCoefficients(float fc, float res) {
        freq = 2.0 * sin(M_PI * std::min(0.25, fc / (sampleRate * 2.0)));
        damp = std::min(2.0*(1.0 - pow(res, 0.25)), std::min(2.0, 2.0 / freq - freq * 0.5));
    }

    void process(float* input, float* output, int samples) {
        for (int i = 0; i < samples; i++) {
            // TODO figure out better upsampling method
            const float in = input[i];
            notch = in - damp * band;
            low   = low + freq * band;
            high  = notch - low;
            band  = freq * high + band - drive * band * band * band;
            output[i] = 0.5 * *out;

            notch = in - damp * band;
            low   = low + freq * band;
            high  = notch - low;
            band  = freq * high + band - drive * band * band * band;
            output[i] += 0.5 * *out;
        }
    }

  private:
    float* out;
    float notch, low, high, band;
    float freq, damp;
    float drive; // internal distortion

    int sampleRate;
};
