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

#include <stdlib.h>
#include <math.h>

float gb(float x) {
    return 2.0 * x - 1.0;
}

float gu(float x) {
    return 0.5 * x + 0.5;
}

float glin(float x, float a1) {
    return a1 * x;
}

float glin(float x, float a1, float a0) {
    return a1 * x + a0;
}

float mod1(float x) {
    return fmod(x, 1.0);
}

float modm(float x, float m) {
    return fmod(x, m);
}

float gramp(float x, float a1, float a0) {
    return mod1(glin(x, a1, a0));
}

float gtri(float x) {
    return fabs(gb(x));
}

float gtri(float x, float a1, float a0) {
    return mod1(glin(fabs(gb(x)),a1,a0));
}

float stri(float x) {
    if (x < 0.5) {
        return 2.0 * x;
    } else {
        return 2.0 - 2.0 * x;
    }
}

float gpulse(float x, float w) {
    return x < w ? 0.0 : 1.0;
}

float gvslope(float x, float w) {
    if (x < w) {
        return x;
    } else {
        return 2.0 * x - 1.0;
    }
}

float svtri(float x, float w) {
    return gb(x) - gb(fabs(x - w));
}

float gvtri(float x, float w, float a1, float a0) {
    return mod1(glin(svtri(x, w), a1, a0));
}

float gripple(float x, float m) {
    return x + fmod(x, m);
}

float gripple2(float x, float m1, float m2) {
    return fmod(x, m1) + fmod(x, m2);
}

/**
 * Phaseshaping Oscillator
 *
 * PhaseShaping uses cascade phase shaping functions to create classic
 * and novel waveforms.
 * Bandlimiting is used where feasible.
 *
 * based on Phaseshaping Oscillator Algorithms for Musical Sound Synthesis
 */

class PhaseShaping {

    enum {SIN, HARD, SOFT, PULSE, SLOPE, TRI, SUPERSAW, SLICES, SINUSOIDS, NOISE};

  public:
    void clear() {
        phase = 0.0;
        freq = 440.0;
        type = SIN;
    }

    void reset() {
        phase = 0.0;
    }

    float sin2(float in) {
        // TODO use lookup table
        return sin(2.0 * M_PI * in);
    }

    float hardsync(float in) {
        return gb(gramp(in, a1, a0));
    }

    float softsync(float in) {
        return gb(stri(gtri(in, a1, a0)));
    }

    float pulse(float in) {
        return gb(gpulse(in, a1));
    }

    float slope(float in) {
        return gb(gvslope(in, a1));
    }

    float jp8000_tri(float in) {
        float p = gb(gtri(in, a1, a0));
        return 2.0 * (p - ceil(p - 0.5));
    }

    float jp8000_supersaw(float in) {
        float m1, m2; // TODO
        float p = gripple2(glin(in, a1), m1, m2);
        return sin2(p);
    }

    float waveslices(float in) {
        return sin2(glin(in, a1));
    }

    float sinusoids(float in) {
        return sin2(gvslope(in, a1));
    }

    float noise() {
        return (double) (2.0 * rand() / (RAND_MAX + 1.0) - 1.0);
    }

    void process(float* output, int samples) {
        float inc = sampleRate / freq;

        for (int i = 0; i < samples; i++) {
            phase += inc;
            if (phase > 1.0) {
                phase -= 1.0;
            }

            // TODO type switch

            // sin
            output[i] = sin2(phase);
            // hard sync
            output[i] = hardsync(phase);
            // soft sync
            output[i] = softsync(phase);
            // pulse
            output[i] = pulse(phase);
            // slope
            output[i] = slope(phase);
            // jp8000 tri
            output[i] = jp8000_tri(phase);
            // jp8000 supersaw
            output[i] = jp8000_supersaw(phase);
            // waveslices
            output[i] = waveslices(phase);
            // sinusoids
            output[i] = sinusoids(phase);
            // noise
            output[i] = noise();
        }
    }

  private:
    float phase, freq, a1, a0;
    int sampleRate, type;

};
