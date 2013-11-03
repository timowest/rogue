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

void ChorusEffect::setCoefficients(float d, float a, float r, float ff, float fb) {
    delay = d;
    amount = a;
    rate = r;
    feedforward = ff;
    feedback = fb;

    lfo_inc = r / sample_rate;
}

void ChorusEffect::setSamplerate(float r) {
    sample_rate = r;
}

void ChorusEffect::process(float* left, float* right, int samples) {
    float dsamples = delay * sample_rate;

    for (uint i = 0; i < samples; i++) {
        // configure
        lfo_phase += lfo_inc;
        if (lfo_phase >= 1.0f) {
            lfo_phase -= 1.0f;
        }
        float lfo_val = sin_.linear(lfo_phase);
        float dl = (1.0 + amount * lfo_val) * dsamples;
        float dr = (1.0 - amount * lfo_val) * dsamples;
        delay_l.setDelay(dl);
        delay_r.setDelay(dr);

        // process
        float dl_out = delay_l.process(left[i] + feedback * last_l);
        float dr_out = delay_r.process(right[i] + feedback * last_r);
        last_l = left[i] + feedforward * dl_out;
        last_r = right[i] + feedforward * dr_out;
        left[i] = last_l;
        right[i] = last_r;
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

// PhaserEffect

void PhaserEffect::clear() {
    last_l = last_r = 0.0;
    lfo_phase = 0.0;

    for (uint i = 0; i < 8; i++) {
        filters_l[i].clear();
        filters_r[i].clear();
    }
}

void PhaserEffect::setCoefficients(float del, float a, float r, float d, float fb) {
    delay = del;
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
    float dsamples = delay * sample_rate;

    for (uint i = 0; i < samples; i++) {
        // configure
        lfo_phase += lfo_inc;
        if (lfo_phase >= 1.0f) {
            lfo_phase -= 1.0f;
        }
        float lfo_val = sin_.linear(lfo_phase);
        float dl = (1.0 + amount * lfo_val) * dsamples;
        float dr = (1.0 - amount * lfo_val) * dsamples;

        // process
        float dl_out = left[i] + feedback * last_l;
        float dr_out = right[i] + feedback * last_r;

        for (uint j = 0; j < 8; j++) {
            filters_l[j].setDelay(dl);
            filters_r[j].setDelay(dr);
            dl_out = filters_l[j].process(dl_out);
            dr_out = filters_r[j].process(dr_out);
        }

        last_l = left[i] + depth * dl_out;
        last_r = right[i] + depth * dr_out;
        left[i] = last_l;
        right[i] = last_r;
    }
}

// DelayEffect

void DelayEffect::clear() {
    last_l = last_r = 0;
    delay_l.clear();
    delay_r.clear();
}

void DelayEffect::setCoefficients(float bpm, float di_l, float di_r, float a, float fb) {
    delay_l.setDelay(sample_rate / (bpm / 60 / di_l));
    delay_r.setDelay(sample_rate / (bpm / 60 / di_r));
    amount = a;
    feedback = fb;
}

void DelayEffect::setSamplerate(float r) {
    sample_rate = r;
    delay_l.setMax(r);
    delay_r.setMax(r);
}

void DelayEffect::process(float* left, float* right, int samples) {
    for (uint i = 0; i < samples; i++) {
        last_l = delay_l.process(left[i] + feedback * last_l);
        last_r = delay_r.process(right[i] + feedback * last_r);
        left[i] += amount * last_l;
        right[i] += amount * last_r;
    }
}


// ReverbEffect

}
