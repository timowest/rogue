/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "effects.h"
#include "tables.h"
#include <stdlib.h>
#include <math.h>

namespace dsp {

// ChorusEffect

void ChorusEffect::clear() {
    delay_l.clear();
    delay_r.clear();
    lfo_phase = 0.0;
    last_l = last_r = 0.0;
}

void ChorusEffect::setCoefficients(float d, float a, float r, float de, float fb) {
    delay = d * sample_rate;
    amount = a;
    rate = r;
    depth = de;
    feedback = fb;

    lfo_inc = r / sample_rate;
}

void ChorusEffect::setSamplerate(float r) {
    sample_rate = r;
}

void ChorusEffect::process(float* left, float* right, int samples) {
    for (uint i = 0; i < samples; i++) {
        // configure
        lfo_phase += lfo_inc;
        if (lfo_phase >= 1.0f) {
            lfo_phase -= 1.0f;
        }
        float lfo_val = sin_.linear(lfo_phase);
        float dl = (1.0 + amount * lfo_val) * delay;
        float dr = (1.0 - amount * lfo_val) * delay;

        // process
        delay_l.setDelay(dl);
        delay_r.setDelay(dr);
        last_l = delay_l.process(left[i] + feedback * last_l);
        last_r = delay_r.process(right[i] + feedback * last_r);
        left[i] += depth * last_l;
        right[i] += depth * last_r;
    }
}

// AllpassDelay

void AllpassDelay::clear() {
    a1 = zm1 = 0;
}

void AllpassDelay::setDelay(float d) {
    a1 = (1.0 - d) / (1.0 + d);
}

float AllpassDelay::process(float in) {
    float y = in * -a1 + zm1;
    zm1 = y * a1 + in;
    return y;
}

float AllpassDelay::process(float a1, float in) {
    float y = in * -a1 + zm1;
    zm1 = y * a1 + in;
    return y;
}

// PhaserEffect

void PhaserEffect::clear() {
    last_l = last_r = 0.0;
    lfo_phase = 0.0;

    for (uint i = 0; i < 8; i++) {
        filters_l[i].clear();
        filters_r[i].clear();
    }
}

void PhaserEffect::setCoefficients(float min_fr, float max_fr, float r, float d, float fb) {
    min_d = 2.0 * min_fr / sample_rate;
    max_d = 2.0 * max_fr / sample_rate;
    delta_d = max_d - min_d;
    rate = r;
    depth = d;
    feedback = fb;

    lfo_inc = r / sample_rate;
}

void PhaserEffect::setSamplerate(float r) {
    sample_rate = r;
}

void PhaserEffect::process(float* left, float* right, int samples) {
    for (uint i = 0; i < samples; i++) {
        // configure
        lfo_phase += lfo_inc;
        if (lfo_phase >= 1.0f) {
            lfo_phase -= 1.0f;
        }
        float lfo_val = 0.5 * (sin_.linear(lfo_phase) + 1);
        float dl = min_d + lfo_val * delta_d;
        float dr = max_d - lfo_val * delta_d;

        // process
        last_l = left[i] + feedback * last_l;
        last_r = right[i] + feedback * last_r;
        float a1l = (1.0 - dl) / (1.0 + dl);
        float a1r = (1.0 - dr) / (1.0 + dr);
        for (uint j = 0; j < 8; j++) {
            last_l = filters_l[j].process(a1l, last_l);
            last_r = filters_r[j].process(a1r, last_r);
        }
        left[i] += depth * last_l;
        right[i] += depth * last_r;
    }
}

// DelayEffect

void DelayEffect::clear() {
    last_l = last_r = 0;
    delay_l.clear();
    delay_r.clear();
    filter_1l.clear();
    filter_2l.clear();
    filter_1r.clear();
    filter_2r.clear();
}

void DelayEffect::setCoefficients(float bpm, float di_l, float di_r, float de, float fb, float pp, float lc, float hc) {
    delay_l.setDelay(sample_rate / (bpm / 60 / di_l));
    delay_r.setDelay(sample_rate / (bpm / 60 / di_r));
    depth = de;
    feedback = fb;
    direct = 1.0 - pp;
    pingpong = pp;

    filter_1l.setHighpass(lc / sample_rate);
    filter_1r.setHighpass(lc / sample_rate);
    filter_2l.setLowpass(hc / sample_rate);
    filter_2r.setLowpass(hc / sample_rate);
}

void DelayEffect::setSamplerate(float r) {
    sample_rate = r;
    delay_l.setMax(2.0 * r); // for 30bpm
    delay_r.setMax(2.0 * r); // for 30bpm
}

void DelayEffect::process(float* left, float* right, int samples) {
    for (uint i = 0; i < samples; i++) {
        float filtered_l = filter_2l.process(filter_1l.process(direct * last_l + pingpong * last_r));
        float filtered_r = filter_2r.process(filter_1r.process(direct * last_r + pingpong * last_l));
        last_l = delay_l.process(left[i] + feedback * filtered_l);
        last_r = delay_r.process(right[i] + feedback * filtered_r);
        left[i] += depth * last_l;
        right[i] += depth * last_r;
    }
}


// ReverbEffect
// Csound orchestra version coded by Sean Costello, October 1999
// C implementation (C) 2005 Istvan Varga

/* reverbParams[n][0] = delay time (in seconds)                     */
/* reverbParams[n][1] = random variation in delay time (in seconds) */
/* reverbParams[n][2] = random variation frequency (in 1/sec)       */
/* reverbParams[n][3] = random seed (0 - 32767)*/
static const double reverbParams[8][4] = {
    { (2473.0 / 44100.0), 0.0010, 3.100,  1966.0 },
    { (2767.0 / 44100.0), 0.0011, 3.500, 29491.0 },
    { (3217.0 / 44100.0), 0.0017, 1.110, 22937.0 },
    { (3557.0 / 44100.0), 0.0006, 3.973,  9830.0 },
    { (3907.0 / 44100.0), 0.0010, 2.341, 20643.0 },
    { (4127.0 / 44100.0), 0.0011, 1.897, 22937.0 },
    { (2143.0 / 44100.0), 0.0017, 0.891, 29491.0 },
    { (1933.0 / 44100.0), 0.0006, 3.221, 14417.0 }
};

ReverbEffect::ReverbEffect() {
    for (uint i = 0; i < 8; i++) {
        adelays[0].setDelay((i & 1) ? 0.6 : -0.6);
    }
}

void ReverbEffect::clear() {
    erDelays[0].clear();
    erDelays[1].clear();

    for (uint i = 0; i < 8; i++) {
        adelays[i].clear();
        delays[i].clear();
        filters[i].clear();
        lfos[i].clear();
    }

    for (uint i = 0; i < 2; i++) {
        highCut[i].clear();
        lowCut[i].clear();
    }
}

void ReverbEffect::setSamplerate(float r) {
    sample_rate = r;

    erDelays[0].setMax(0.12 * r);
    erDelays[1].setMax(0.12 * r);

    for (uint i = 0; i < 8; i++) {
        delays[i].setMax(8192);
        lfos[i].setSamplerate(r);
        lfos[i].setType(0); // sine
        lfos[i].setFreq(reverbParams[i][2]);
    }
}

void ReverbEffect::setCoefficients(float _pre, float _decay, float _lowCut, float _highCut, float _depth) {
    erDelays[0].setDelay(_pre * sample_rate);
    erDelays[1].setDelay(_pre * sample_rate);

    gain = sqrt(_decay);
    pitchmod = 0.6;
    tone = 0.9; // TODO make this also configurable
    depth = _depth;

    for (uint i = 0; i < 8; i++) {
        filters[i].setLowpass(tone);
    }

    _lowCut /= sample_rate;
    lowCut[0].setHighpass(_lowCut, 0);
    lowCut[1].setHighpass(_lowCut, 0);

    _highCut /= sample_rate;
    highCut[0].setLowpass(_highCut, 0);
    highCut[1].setLowpass(_highCut, 0);
}

void ReverbEffect::process(float* left, float* right, int samples) {
    for (uint i = 0; i < samples; i++) {
        // pre delay
        float pleft = erDelays[0].process(left[i]);
        float pright = erDelays[1].process(right[i]);

        // pre filters
        float fleft = highCut[0].process(pleft);
        float fright = highCut[1].process(pright);
        fleft = lowCut[0].process(fleft);
        fright = lowCut[1].process(fright);

        // calculate junction pressure
        float apj = 0.0;
        for (uint j = 0; j < 8; j++) apj += filters[j].getLast();
        apj *= 0.25;

        // FDN delay lines
        float l = fleft + apj;
        float r = fright + apj;
        for (uint j = 0; j < 8; j++) {
            float d = reverbParams[j][0] + lfos[j].tick() * pitchmod * reverbParams[j][1];
            delays[j].setDelay(sample_rate * d);
            // send input signal and feedback to delay line
            float fb = filters[j].getLast();
            float noise = (2.0f * rand() / (RAND_MAX + 1.0f) - 1.0f);
            fb += fb * 0.001 * noise;
            float aout = adelays[j].process((j & 1 ? r : l) - fb);
            filters[j].process(gain * delays[j].process(aout));
        }

        // mix
        float lout = filters[0].getLast() + filters[2].getLast()
                   + filters[4].getLast() + filters[6].getLast();
        float rout = filters[1].getLast() + filters[3].getLast()
                   + filters[5].getLast() + filters[7].getLast();

        float dry = 1.0f - depth;
        left[i] = dry * left[i] + depth * lout;
        right[i] = dry * right[i] + depth * rout;
    }
}

}
