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

float gb(float x) {
    return 2.0f * x - 1.0f;
}

float gu(float x) {
    return 0.5f * x + 0.5f;
}

float glin(float x, float a1) {
    return a1 * x;
}

float glin(float x, float a1, float a0) {
    return a1 * x + a0;
}

float mod1(float x) {
    return fmod(x, 1.0f);
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
    return mod1(glin(fabs(gb(x)), a1, a0));
}

float stri(float x) {
    if (x < 0.5f) {
        return 2.0f * x;
    } else {
        return 2.0f - 2.0f * x;
    }
}

float gpulse(float x, float w) {
    return x < w ? 0.0f : 1.0f;
}

float gvslope(float x, float w) {
    return x < w ? x : (x - w) / (1.0f - w);
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

}

#endif
