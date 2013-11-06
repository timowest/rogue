/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "effects.h"
#include "tables.h"

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

void PhaserEffect::setCoefficients(float fr, float a, float r, float d, float fb) {
    delay = 2.0 * fr / sample_rate;
    amount = a;
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
        float lfo_val = sin_.linear(lfo_phase);
        float dl = (1.0 + amount * lfo_val) * delay;
        float dr = (1.0 - amount * lfo_val) * delay;

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

void DelayEffect::setCoefficients(float bpm, float di_l, float di_r, float de, float fb, float lc, float hc) {
    delay_l.setDelay(sample_rate / (bpm / 60 / di_l));
    delay_r.setDelay(sample_rate / (bpm / 60 / di_r));
    depth = de;
    feedback = fb;

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
        float filtered_l = filter_2l.process(filter_1l.process(last_l));
        float filtered_r = filter_2r.process(filter_1r.process(last_r));
        last_l = delay_l.process(left[i] + feedback * filtered_l);
        last_r = delay_r.process(right[i] + feedback * filtered_r);
        left[i] += depth * last_l;
        right[i] += depth * last_r;
    }
}


// ReverbEffect

// TODO

}
