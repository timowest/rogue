/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * based on synthv1 (http://synthv1.sourceforge.net/)
 * Copyright 2013 Rui Nuno Capela
 */

#include "effects.h"
#include "tables.h"
#include <stdlib.h>
#include <math.h>

#define SIN(x) sin_.linear(x)

namespace dsp {

// Flanger

void Flanger::reset() {
    for (int i = 0; i < MAX_SIZE; ++i) {
        buffer[i] = 0.0f;
    }
    frames = 0;
}

float Flanger::output(float in, float delay, float feedb) {
    // calculate delay offset
    float delta = float(frames) - delay;
    // clip lookback buffer-bound
    if (delta < 0.0f) {
        delta += float(MAX_SIZE);
    }
    // get index
    int index = int(delta);
    // 4 samples hermite
    float y0 = buffer[(index + 0) & MAX_MASK];
    float y1 = buffer[(index + 1) & MAX_MASK];
    float y2 = buffer[(index + 2) & MAX_MASK];
    float y3 = buffer[(index + 3) & MAX_MASK];
    // csi calculate
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    // compute interpolation x
    float x = delta - floorf(delta);
    // get output
    float out = ((c3 * x + c2) * x + c1) * x + c0;
    // add to delay buffer
    buffer[(frames++) & MAX_MASK] = in + out * feedb;
    // return output
    return out;
}

// Chorus

void Chorus::reset() {
    flanger1.reset();
    flanger2.reset();
    lfo_phase = 0.0f;
}

void Chorus::process(float* l, float* r, int samples) {
    // constrained feedback
    float feedb = feedback * 0.95f;
    // calculate delay time
    float d0 = 0.5f * delay * float(4096);
    float a1 = 0.99f * d0 * mod * mod;
    float r2 = 2.0f * rate * rate / sample_rate;

    // process
    for (int i = 0; i < samples; i++) {
        // modulation
        float lfo = a1 * SIN(lfo_phase);
        float delay1 = d0 - lfo;
        float delay2 = d0 - lfo * 0.9f;
        // chorus mix
        l[i] += wet * flanger1.output(l[i], delay1, feedb);
        r[i] += wet * flanger2.output(r[i], delay2, feedb);
        // lfo advance
        lfo_phase += r2;
        // lfo wrap
        if (lfo_phase > 1.0f) {
            lfo_phase -= 1.0f;
        }
    }
}

// Phaser


void Phaser::reset() {
    // initialize vars
    lfo_phase = 0.0f;
    out_l = 0.0f;
    out_r = 0.0f;
    // reset taps
    for (int n = 0; n < 6; ++n) {
        ap_l[n].reset();
        ap_r[n].reset();
    }
}

void Phaser::process(float* l, float* r, int samples) {
    float delay_min = 2.0f * 440.0f / sample_rate;
    float delay_max = 2.0f * 4400.0f / sample_rate;
    float lfo_inc = lfo_rate / sample_rate;

    for (int i = 0; i < samples; i++) {
        // calculate and update phaser lfo
        float delay = delay_min + (delay_max - delay_min) * 0.5f * (1.0f + SIN(lfo_phase));
        float a = (1.0 - delay) / (1.0 + delay);
        // increment phase
        lfo_phase += lfo_inc;
        if (lfo_phase >= 1.0f) {
            lfo_phase -= 1.0f;
        }

        // anti-denormalizer noise
        float ad = 1E-14f * float(rand());
        // get input
        out_l = l[i] + ad + out_l * feedback;
        out_r = r[i] + ad + out_r * feedback;
        // update filter coeffs and calculate output
        for (int n = 0; n < 6; n++) {
            ap_l[n].set(a);
            ap_r[n].set(a);
            out_l = ap_l[n].process(out_l);
            out_r = ap_l[n].process(out_r);
        }
        // output
        l[i] += wet * out_l * depth;
        r[i] += wet * out_r * depth;
    }
}

// Delay

void Delay::reset() {
    for (int i = 0; i < MAX_SIZE; i++) {
        buffer_l[i] = 0.0f;
        buffer_r[i] = 0.0f;
    }
    out_l = out_r = 0.0f;
    frames = 0;
}

void Delay::process(float* l, float* r, int samples) {
    if (wet < 1E-9f) {
        return;
    }
    // calculate delay time
    float delay_time = delay * sample_rate;
    if (bpm > 0.0f) {
        delay_time *= 60.f / bpm;
    }
    // set integer delay
    int ndelay = int(delay_time);
    // clamp
    if (ndelay < 256) {
        ndelay = 256;
    }
    if (ndelay > MAX_SIZE) {
        ndelay = MAX_SIZE;
    }
    // delay process
    for (int i = 0; i < samples; ++i) {
        int j = (frames++) & MAX_MASK;
        out_l = buffer_l[(j - ndelay) & MAX_MASK];
        out_r = buffer_r[(j - ndelay) & MAX_MASK];
        buffer_l[j] = *l + out_l * feedb;
        buffer_r[j] = *r + out_r * feedb;
        *l++ += wet * out_l;
        *r++ += wet * out_r;
    }
}

// Reverb

void Reverb::process(float* l, float* r, int samples) {
    // TODO
}

}

