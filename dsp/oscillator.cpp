/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "oscillator.h"
#include "phase.h"
#include "tables.h"

#define SIN(x) sin_.linear(x)
#define COS(x) cos_.linear(x)
#define CASE(a,b) case a: b(output, samples); break;

namespace dsp {

static float polyblep(float t) {
    if (t > 0.0f) {
        return t - (t*t)/2.0f - 0.5f;
    } else {
        return (t*t)/2.0f + t + 0.5f;
    }
}

static float pd(float x, float w) {
    if (x < w) {
        return 0.5f * x / w;
    } else {
        return 0.5f + 0.5f * (x - w) / (1.0f - w);
    }
}

// VA

void VA::highpass(float* output, int samples) {
    float b = 2.0f - COS((0.602f * freq) / sample_rate);
    float c2 = b - sqrt(b*b - 1.0f);

    for (int i = 0; i < samples; i++) {
        float x = output[i];
        output[i] = c2 * (prev + x);
        prev = output[i] - x;
    }
}

void VA::saw(float* output, int samples) {
    // saw
    el.setType(0);
    el.setFreq(freq);
    el.setParams(tone, wf, wt);
    el.process(output, samples);

    // highpass
    highpass(output, samples);
}

void VA::tri_saw(float* output, int samples) {
    // tri
    el.setType(2);
    el.setFreq(freq);
    el.setParams(tone, wf, wt);
    el.process(output, samples);

    // highpass
    highpass(output, samples);
}

void VA::pulse(float* output, int samples) {
    // pulse
    el.setType(5);
    el.setFreq(freq);
    el.setParams(tone, wf, wt);
    el.process(output, samples);

    // highpass
    highpass(output, samples);
}

void VA::process(float* output, int samples) {
    switch (type) {
    CASE(SAW, saw)
    CASE(TRI_SAW, tri_saw)
    CASE(PULSE, pulse)
    }
}

// PD

void PD::saw(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 0.5f - wf * 0.5;
    float m_step = 0.5f * (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        output[i] = COS(pd(phase, mod));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::square(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 0.5f - wf * 0.5;
    float m_step = 0.5f * (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2;
        if (phase < mod) {
            p2 = phase * 0.5f / mod;
        } else if (phase < 0.5f) {
            p2 = 0.5f;
        } else if (phase < 0.5f + mod) {
            p2 = (phase - 0.5f) * 0.5f / mod + 0.5f;
        } else {
            p2 = 1.0f;
        }
        output[i] = COS(p2);
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::pulse(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = phase < mod ? phase / mod : 1.0f;
        output[i] = COS(p2);
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::double_sine(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = 0;
        if (phase < 0.5f) {
            p2 = 2.0f * phase;
        } else {
            p2 = 1.0f - (phase - 0.5f) / (0.5f * mod);
            if (p2 < 0) p2 = 0;
        }
        output[i] = COS(p2);
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::saw_pulse(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = 0;
        if (phase < 0.5f) {
            p2 = phase;
        } else {
            p2 = 0.5f - (phase - 0.5f) / mod;
            if (p2 < 0) p2 = 0;
        }
        output[i] = COS(p2);
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::res1(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = fmod(mod * phase, 1.0f);
        float window = 1.0f - phase;
        output[i] = 1.0f - window * (1.0 - COS(p2));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::res2(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = fmod(mod * phase, 1.0f);
        float window = phase < 0.5f ? 2.0f * phase : 2.0f * (1.0f - phase);
        output[i] = 1.0f - window * (1.0 - COS(p2));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::res3(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = fmod(mod * phase, 1.0f);
        float window = phase < 0.5f ? 1.0f : 2.0f * (1.0f - phase);
        output[i] = 1.0f - window * (1.0 - COS(p2));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::half_sine(float* output, int samples) {
    float inc = freq / sample_rate;
    float mod = 0.5f + wf * 0.5;
    float m_step = 0.5f * (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        output[i] = gb(SIN(0.5f * pd(phase, mod)));
        phase = fmod(phase + inc, 1.0f);
        mod += m_step;
    }
}

void PD::process(float* output, int samples) {
    switch (type) {
    CASE(SAW, saw)
    CASE(SQUARE, square)
    CASE(PULSE, pulse)
    CASE(DOUBLE_SINE, double_sine)
    CASE(SAW_PULSE, saw_pulse)
    CASE(RES1, res1)
    CASE(RES2, res2)
    CASE(RES3, res3)
    CASE(HALF_SINE, half_sine)
    }
}

// EL

void EL::saw(float* output, int samples) {
    float inc = freq / sample_rate;

    for (int i = 0; i < samples; i++) {
        float mod = 0.0f;
        if (phase < inc) { // start
            mod = polyblep(phase / inc);
        } else if (phase > (1.0f - inc)) { // end
            mod = polyblep( (phase - 1.0) / inc);
        }
        output[i] = gb(phase - mod);
        phase = fmod(phase + inc, 1.0f);
    }
}

void EL::double_saw(float* output, int samples) {
    float inc = freq / sample_rate;
    float width = wf;
    float w_step = (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2;
        float mod = 0.0f;
        if (phase < width) {
            float inc2 = inc / width;
            p2 = phase / width;

            if (p2 < inc2) { // start
                mod = polyblep(p2 / inc2);
            } else if (p2 > (1.0f - inc2)) { // end
                mod = polyblep((p2 - 1.0f) / inc2);
            }
        } else {
            float inc2 = inc / (1.0f - width);
            p2 = (phase - width) / (1.0f - width);

            if (p2 < inc2) {
                mod = polyblep(p2 / inc2);
            } else if (p2 > (1.0f - inc2)) {
                mod = polyblep((p2 - 1.0f) / inc2);
            }
        }
        output[i] = gb(p2 - mod);
        phase = fmod(phase + inc, 1.0f);
        width += w_step;
    }
}

void EL::tri(float* output, int samples) {
    float inc = freq / sample_rate;
    float width = wf;
    float w_step = (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        output[i] = gb(gtri(phase, width));
        phase = fmod(phase + inc, 1.0f);
        width += w_step;
    }
}

void EL::tri2(float* output, int samples) {
    float inc = freq / sample_rate;
    float width = wf;
    float w_step = (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p = gtri(phase, width);
        output[i] = gb(p * sqrt(p) + p * (1.0-p));
        phase = fmod(phase + inc, 1.0f);
        width += w_step;
    }
}

void EL::tri3(float* output, int samples) {
    float inc = freq / sample_rate;
    float width = wf;
    float w_step = (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p = gtri(phase, width);
        output[i] = gb(phase < width ? sqrt(p) : p*p*p);
        phase = fmod(phase + inc, 1.0f);
        width += w_step;
    }
}

void EL::pulse(float* output, int samples) {
    float inc = freq / sample_rate;
    bool bl = wf > inc && wf < (1.0f - inc);
    float width = wf;
    float w_step = (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2;
        float mod = 0.0f;

        if (phase < width) {
            p2 = 0.0f;
            if (bl) {
                if (phase < inc) { // start
                    mod = polyblep(phase / inc);
                } else if (phase > (width - inc)) {
                    mod = -polyblep( (phase - width) / inc);
                }
            }
        } else {
            p2 = 1.0f;
            if (bl) {
                if (phase > (1.0f - inc)) { // end
                    mod = polyblep( (phase - 1.0f) / inc);
                } else if (phase < (width + inc)) {
                    mod = -polyblep( (phase - width) / inc);
                }
            }
        }

        output[i] = gb(p2 - mod);
        phase = fmod(phase + inc, 1.0f);
        width += w_step;
    }
}

// TODO polyblep
void EL::pulse_saw(float* output, int samples) {
    float inc = freq / sample_rate;
    float width = wf;
    float w_step = (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = pd(phase, width);
        if (phase < width) {
            output[i] = 2.0f * p2;
        } else {
            output[i] = -2.0f * (p2 - 0.5f);
        }
        phase = fmod(phase + inc, 1.0f);
        width += w_step;
    }
}

// TODO optimize
void EL::slope(float* output, int samples) {
    // bandlimited saw
    float inc = freq / sample_rate;
    float width = wf;
    float w_step = (wt - wf) / (float)samples;

    for (int i = 0; i < samples; i++) {
        float p2 = gvslope(phase, width);
        float mod = 0.0f;

        float inc2 = inc / (1.0f - width);
        if (phase < inc) {               // start
            mod = polyblep(phase / inc);
        } else if (p2 > (1.0f - inc2)) { // end
            mod = polyblep( (p2 - 1.0f) / inc2);
        } else if (phase < width && phase > (width - inc)) {
            mod = width * polyblep( (p2 - width) / inc);
        } else if (phase > width && p2 < inc2) {
            mod = width * polyblep(p2 / inc2);
        }

        output[i] = gb(p2 - mod);
        phase = fmod(phase + inc, 1.0f);
        width += w_step;
    }
}

void EL::alpha1(float* output, int samples) {
    // pulse
    float f = freq;
    float p = phase;
    freq = 2.0f * freq;
    pulse(output, samples);

    // saw
    phase = p;
    freq = f;
    float inc = freq / sample_rate;
    for (int i = 0; i < samples; i++) {
        output[i] = phase * (output[i] + 1.0) - 1.0f;
        phase = fmod(phase + inc, 1.0f);
    }
}

void EL::alpha2(float* output, int samples) {
    // pulse
    float f = freq;
    float p = phase;
    freq = 4.0f * freq;
    pulse(output, samples);

    // saw
    phase = p;
    freq = f;
    float inc = freq / sample_rate;
    for (int i = 0; i < samples; i++) {
        output[i] = phase * (output[i] + 1.0) - 1.0f;
        phase = fmod(phase + inc, 1.0f);
    }
}

void EL::process(float* output, int samples) {
    switch (type) {
    CASE(SAW, saw)
    CASE(DOUBLE_SAW, double_saw)
    CASE(TRI, tri)
    CASE(TRI2, tri2)
    CASE(TRI3, tri3)
    CASE(PULSE, pulse)
    CASE(PULSE_SAW, pulse_saw)
    CASE(SLOPE, slope)
    CASE(ALPHA1, alpha1)
    CASE(ALPHA2, alpha2)
    }
}

// AS

void AS::saw(float* output, int samples) {
    float inc = freq / sample_rate;
    // TODO take sr into account
    float max = 20.0f * tone;

    for (int i = 0.; i < samples; i++) {
        float y = 0.0f;
        for (float j = 1; j < max; j++) {
            y += SIN(fmod(j * phase, 1.0f)) * 1.0/j;
        }
        output[i] = -2.0f/M_PI * y;
        phase = fmod(phase + inc, 1.0f);
    }
}

void AS::square(float* output, int samples) {
    float inc = freq / sample_rate;
    // TODO take sr into account
    float max = 40.0f * tone;

    for (int i = 0; i < samples; i++) {
        float y = 0.0f;
        for (float j = 1; j < max; j += 2.0f) {
            y += SIN(fmod(j * phase, 1.0f)) * 1.0/j;
        }
        output[i] = 4.0/M_PI * y;
        phase = fmod(phase + inc, 1.0f);
    }
}

void AS::impulse(float* output, int samples) {
    float inc = freq / sample_rate;
    // TODO take sr into account
    float max = 20.0f * tone;

    for (int i = 0; i < samples; i++) {
        float y = 0.0f;
        for (float j = 1; j < max; j++) {
            y += SIN(fmod(j * phase, 1.0f));
        }
        // TODO take max into account
        output[i] = 0.05 * y;
        phase = fmod(phase + inc, 1.0f);
    }
}

void AS::process(float* output, int samples) {
    switch (type) {
    CASE(SAW, saw)
    CASE(SQUARE, square)
    CASE(IMPULSE, impulse)
    }
}

// Noise

void Noise::process(float* output, int samples) {
    for (int i = 0; i < samples; i++) {
        output[i] =  (2.0f * rand() / (RAND_MAX + 1.0f) - 1.0f);
    }
}

// TODO pink noise

// TODO brown noise



}
