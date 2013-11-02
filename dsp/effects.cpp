/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "effects.h"

namespace dsp {

// ChorusEffect

void ChorusEffect::clear() {
    delay_l.clear();
    delay_r.clear();
    lfo_l.clear();
    lfo_r.clear();

    last_l = last_r = 0.0;
}

void ChorusEffect::setCoefficients(float d, float a, float r, float ff, float fb) {
    delay = d;
    amount = a;
    rate = r;
    feedforward = ff;
    feedback = fb;

    lfo_l.setFreq(r);
    lfo_r.setFreq(r);
}

void ChorusEffect::setSamplerate(float r) {
    sample_rate = r;
    lfo_l.setSamplerate(r);
    lfo_r.setSamplerate(r);
}

void ChorusEffect::process(float* left, float* right, int samples) {
    float dsamples = delay * sample_rate;

    for (uint i = 0; i < samples; i++) {
        // configure
        float ll = lfo_l.tick();
        float lr = lfo_r.tick();
        float dl = (1.0 + amount * ll) * dsamples;
        float dr = (1.0 + amount * lr) * dsamples;
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

    lfo_l.setFreq(r);
    lfo_r.setFreq(r);
}

void PhaserEffect::setSamplerate(float r) {
    sample_rate = r;
    lfo_l.setSamplerate(r);
    lfo_r.setSamplerate(r);
}

void PhaserEffect::process(float* left, float* right, int samples) {
    float dsamples = delay * sample_rate;

    float ll = lfo_l.tick(samples);
    float lr = lfo_r.tick(samples);
    float dl = (1.0 + amount * ll) * dsamples;
    float dr = (1.0 + amount * lr) * dsamples;

    for (uint i = 0; i < 8; i++) {
        filters_l[i].setDelay(dl);
        filters_r[i].setDelay(dr);
    }

    for (uint i = 0; i < samples; i++) {
        float dl_out = left[i] + feedback * last_l;
        float dr_out = right[i] + feedback * last_r;

        for (uint j = 0; j < 8; j++) {
            dl_out = filters_l[i].process(dl_out);
            dr_out = filters_r[i].process(dr_out);
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

void DelayEffect::setCoefficients(float bpm, float di, float a, float fb) {
    delay = sample_rate / (bpm / 60 / di);
    amount = a;
    feedback = fb;
}

void DelayEffect::setSamplerate(float r) {
    sample_rate = r;
    delay_l.setMax(r);
    delay_r.setMax(r);
}

void DelayEffect::process(float* left, float* right, int samples) {
    delay_l.setDelay(delay);
    delay_r.setDelay(delay);

    for (uint i = 0; i < samples; i++) {
        last_l = delay_l.process(left[i] + feedback * last_l);
        last_r = delay_r.process(right[i] + feedback * last_r);
        left[i] += amount * last_l;
        right[i] += amount * last_r;
    }
}


// ReverbEffect

}
