/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_PHASE_H
#define DSP_PHASE_H

#include <stdlib.h>
#include <math.h>

namespace dsp {

static float gb(float x) {
    return 2.0f * x - 1.0f;
}

static float gu(float x) {
    return 0.5f * x + 0.5f;
}

static float glin(float x, float a1) {
    return a1 * x;
}

static float glin(float x, float a1, float a0) {
    return a1 * x + a0;
}

static float mod1(float x) {
    return fmod(x, 1.0f);
}

static float modm(float x, float m) {
    return fmod(x, m);
}

static float gramp(float x, float a1, float a0) {
    return mod1(glin(x, a1, a0));
}

static float gtri(float x) {
    return fabs(gb(x));
}

static float gtri(float x, float w) {
    if (x < w) {
        return x / w;
    } else {
        return 1.0 - (x-w) / (1.0-w);
    }
}

static float gtri(float x, float a1, float a0) {
    return mod1(glin(fabs(gb(x)), a1, a0));
}

static float stri(float x) {
    if (x < 0.5f) {
        return 2.0f * x;
    } else {
        return 2.0f - 2.0f * x;
    }
}

static float gpulse(float x, float w) {
    return x < w ? 0.0f : 1.0f;
}

static float gvslope(float x, float w) {
    return x < w ? x : (x - w) / (1.0f - w);
}

static float svtri(float x, float w) {
    return gb(x) - gb(fabs(x - w));
}

static float gvtri(float x, float w, float a1, float a0) {
    return mod1(glin(svtri(x, w), a1, a0));
}

static float gripple(float x, float m) {
    return x + fmod(x, m);
}

static float gripple2(float x, float m1, float m2) {
    return fmod(x, m1) + fmod(x, m2);
}

}

#endif
