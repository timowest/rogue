/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include "oscillator.h"
#include <algorithm>
#include "polyblep.h"
#include "phase.h"
#include "tables.h"
#include "types.h"

#define SIN(x) sin_.linear(x)

#define COS(x) cos_.linear(x)

#define CASE(a,b) case a: b(output, out_sync, samples); break;

// sync  >= 0.0  samples after reset
//       > -1.0  samples before reset
//       -2.0    no reset

#define INC_PHASE_SYNC() \
    phase_ = phase; \
    phase += inc; \
    if (input_sync[i] >= 0.0) { \
        phase = input_sync[i] * inc; \
        out_sync[i] = phase / inc; \
    }  else if (phase >= 1.0f) { \
        phase -= 1.0f; \
        out_sync[i] = phase / inc; \
    } else if (phase > (1.0 - inc)) { \
        out_sync[i] = -(1.0 - phase) / inc; \
    } else { \
        out_sync[i] = input_sync[i]; \
    }

#define INC_PHASE() \
    phase_ = phase; \
    phase += inc; \
    if (phase >= 1.0f) { \
        phase -= 1.0f; \
        out_sync[i] = phase / inc; \
    } else if (phase > (1.0 - inc)) { \
        out_sync[i] = -(1.0 - phase) / inc; \
    } else { \
        out_sync[i] = -2.0; \
    }

// phase loop

#define PHASE_LOOP_SYNC(calc) \
    float inc = freq / sample_rate; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE_SYNC() \
        calc \
    }

#define PHASE_LOOP(calc) \
    float inc = freq / sample_rate; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE() \
        calc \
    }

#define PHASE_LOOP_PM_SYNC(calc) \
    float inc = freq / sample_rate; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE_SYNC() \
        float phase = pmod(this->phase, i); \
        calc \
    }

#define PHASE_LOOP_PM(calc) \
    float inc = freq / sample_rate; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE() \
        float phase = pmod(this->phase, i); \
        calc \
    }

#define PHASE_LOOP_BOTH(calc) \
    if (pm > 0.0f) { \
        if (sync) { \
            PHASE_LOOP_PM_SYNC(calc) \
        } else { \
            PHASE_LOOP_PM(calc) \
        } \
    } else if (sync) { \
        PHASE_LOOP_SYNC(calc) \
    } else { \
        PHASE_LOOP(calc) \
    }

// pwdith loop

#define PWIDTH_LOOP_SYNC(calc) \
    float inc = freq / sample_rate; \
    float width = norm_width(wf, inc); \
    float w_step = (norm_width(wt, inc) - width) / (float)samples; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE_SYNC() \
        calc \
        width += w_step; \
    }

#define PWIDTH_LOOP(calc) \
    float inc = freq / sample_rate; \
    float width = norm_width(wf, inc); \
    float w_step = (norm_width(wt, inc) - width) / (float)samples; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE() \
        calc \
        width += w_step; \
    }

#define PWIDTH_LOOP_PM_SYNC(calc) \
    float inc = freq / sample_rate; \
    float width = norm_width(wf, inc); \
    float w_step = (norm_width(wt, inc) - width) / (float)samples; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE_SYNC() \
        float phase = pmod(this->phase, i); \
        calc \
        width += w_step; \
    }

#define PWIDTH_LOOP_PM(calc) \
    float inc = freq / sample_rate; \
    float width = norm_width(wf, inc); \
    float w_step = (norm_width(wt, inc) - width) / (float)samples; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE() \
        float phase = pmod(this->phase, i); \
        calc \
        width += w_step; \
    }

#define PWIDTH_LOOP_BOTH(calc) \
    if (pm > 0.0f) { \
        if (sync) { \
            PWIDTH_LOOP_PM_SYNC(calc) \
        } else { \
            PWIDTH_LOOP_PM(calc) \
        } \
     } else if (sync) { \
        PWIDTH_LOOP_SYNC(calc) \
    } else { \
        PWIDTH_LOOP(calc) \
    }

// pmod loop

#define PMOD_LOOP_SYNC(calc) \
    float inc = freq / sample_rate; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE_SYNC() \
        calc \
        mod += m_step; \
    }

#define PMOD_LOOP(calc) \
    float inc = freq / sample_rate; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE() \
        calc \
        mod += m_step; \
    }

#define PMOD_LOOP_PM_SYNC(calc) \
    float inc = freq / sample_rate; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE_SYNC() \
        float phase = pmod(this->phase, i); \
        calc \
        mod += m_step; \
    }

#define PMOD_LOOP_PM(calc) \
    float inc = freq / sample_rate; \
    for (uint i = 0; i < samples; i++) { \
        INC_PHASE() \
        float phase = pmod(this->phase, i); \
        calc \
        mod += m_step; \
    }

#define PMOD_LOOP_BOTH(calc) \
    if (pm > 0.0f) { \
        if (sync) { \
            PMOD_LOOP_PM_SYNC(calc) \
        } else { \
            PMOD_LOOP_PM(calc) \
        } \
    } else if (sync) { \
        PMOD_LOOP_SYNC(calc) \
    } else { \
        PMOD_LOOP(calc) \
    }

namespace dsp {

/** phase distortion */
static float pd(float x, float w) {
    if (x < w) {
        return 0.5f * x / w;
    } else {
        return 0.5f + 0.5f * (x - w) / (1.0f - w);
    }
}

static float norm_width(float w, float inc) {
    if (w < inc) {
        return inc;
    } else if (w > (1.0 - inc)) {
        return 1.0 - inc;
    } else {
        return w;
    }
}

// VA

void Virtual::va_saw(float* output, float* out_sync, int samples) {
    el_saw(output, out_sync, samples);
}

void Virtual::va_tri_saw(float* output, float* out_sync, int samples) {
    el_tri(output, out_sync, samples);
}

void Virtual::va_pulse(float* output, float* out_sync, int samples) {
    el_pulse(output, out_sync, samples);
}

// PD

void Virtual::pd_saw(float* output, float* out_sync, int samples) {
    float mod = 0.5f - wf * 0.5;
    float m_step = 0.5f * (wf - wt) / (float)samples;

    PMOD_LOOP_BOTH(
        output[i] = COS(pd(phase, mod));
    )
}

void Virtual::pd_square(float* output, float* out_sync, int samples) {
    float mod = 0.5f - wf * 0.5;
    float m_step = 0.5f * (wf - wt) / (float)samples;

    PMOD_LOOP_BOTH(
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
    )
}

void Virtual::pd_pulse(float* output, float* out_sync, int samples) {
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    PMOD_LOOP_BOTH(
        float p2 = phase < mod ? phase / mod : 1.0f;
        output[i] = COS(p2);
    )
}

void Virtual::pd_double_sine(float* output, float* out_sync, int samples) {
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    PMOD_LOOP_BOTH(
        float p2 = 0;
        if (phase < 0.5f) {
            p2 = 2.0f * phase;
        } else {
            p2 = 1.0f - (phase - 0.5f) / (0.5f * mod);
            if (p2 < 0) p2 = 0;
        }
        output[i] = COS(p2);
    )
}

void Virtual::pd_saw_pulse(float* output, float* out_sync, int samples) {
    float mod = 1.0f - wf;
    float m_step = (wf - wt) / (float)samples;

    PMOD_LOOP_BOTH(
        float p2 = 0.0f;
        if (phase < 0.5f) {
            p2 = phase;
        } else {
            p2 = 0.5f - (phase - 0.5f) / mod;
            if (p2 < 0) p2 = 0;
        }
        output[i] = COS(p2);
    )
}

void Virtual::pd_res1(float* output, float* out_sync, int samples) {
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    PMOD_LOOP_BOTH(
        float p2 = fmod(mod * phase, 1.0f);
        float window = 1.0f - phase;
        output[i] = 1.0f - window * (1.0 - COS(p2));
    )
}

void Virtual::pd_res2(float* output, float* out_sync, int samples) {
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    PMOD_LOOP_BOTH(
        float p2 = fmod(mod * phase, 1.0f);
        float window = phase < 0.5f ? 2.0f * phase : 2.0f * (1.0f - phase);
        output[i] = 1.0f - window * (1.0 - COS(p2));
    )
}

void Virtual::pd_res3(float* output, float* out_sync, int samples) {
    float mod = expf(wf * 6.0f * (float)M_LN2);
    float modt = expf(wt * 6.0f * (float)M_LN2);
    float m_step = (modt - mod) / (float)samples;

    PMOD_LOOP_BOTH(
        float p2 = fmod(mod * phase, 1.0f);
        float window = phase < 0.5f ? 1.0f : 2.0f * (1.0f - phase);
        output[i] = 1.0f - window * (1.0 - COS(p2));
    )
}

void Virtual::pd_half_sine(float* output, float* out_sync, int samples) {
    float mod = 0.5f + wf * 0.5;
    float m_step = 0.5f * (wt - wf) / (float)samples;

    PMOD_LOOP_BOTH(
        output[i] = gb(SIN(0.5f * pd(phase, mod)));
    )
}

// EL

// polyblep
void Virtual::el_saw(float* output, float* out_sync, int samples) {
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                output[i] = gb(phase);
            )
        } else {
            PHASE_LOOP_PM(
                output[i] = gb(phase);
            )
        }
    } else if (sync) {
        // bandlimited
        PHASE_LOOP_SYNC(
            float mod = saw_sync(phase, phase_, inc, input_sync[i]);
            output[i] = gb(phase - mod);
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            float mod = saw_polyblep(phase, inc);
            output[i] = gb(phase - mod);
        )
    }
}

static float double_saw(float phase, float width) {
    if (phase < width) {
        return phase / width;
    } else {
        return (phase - width) / (1.0f - width);
    }
}

// polyblep
void Virtual::el_double_saw(float* output, float* out_sync, int samples) {
    if (pm > 0.0) {
        if (sync) {
            PWIDTH_LOOP_PM_SYNC(
                output[i] = gb(double_saw(phase, width));
            )
        } else {
            PWIDTH_LOOP_PM(
                output[i] = gb(double_saw(phase, width));
            )
        }
    } else if (sync) {
        // bandlimited
        PWIDTH_LOOP_SYNC(
            float p2 = double_saw(phase, width);
            float s = input_sync[i];
            float mod = 0.0f;
            if (s >= 0.0f) { // sync start
                mod = double_saw(phase_, width) * polyblep(s);
            } else if (s > -1.0f) { // sync end
                mod = double_saw(phase, width) * polyblep(s);
            } else if (phase < inc) { // start
                mod = polyblep(phase / inc);
            } else if (phase > (1.0f - inc)) { // end
                mod = polyblep( (phase - 1.0f) / inc);
            } else if (phase < width && phase > (width - inc)) { // mid end
                mod = polyblep( (phase - width) / inc);
            } else if (phase > width && phase < (width + inc)) { // mid start
                mod = polyblep((phase - width) / inc);
            }
            output[i] = gb(p2 - mod);
        )
    } else {
        // bandlimited
        PWIDTH_LOOP(
            float p2 = double_saw(phase, width);
            float mod = 0.0f;
            if (phase < inc) { // start
                mod = polyblep(phase / inc);
            } else if (phase > (1.0f - inc)) { // end
                mod = polyblep( (phase - 1.0f) / inc);
            } else if (phase < width && phase > (width - inc)) { // mid end
                mod = polyblep( (phase - width) / inc);
            } else if (phase > width && phase < (width + inc)) { // mid start
                mod = polyblep((phase - width) / inc);
            }
            output[i] = gb(p2 - mod);
        )
    }
}

void Virtual::el_tri(float* output, float* out_sync, int samples) {
    if (pm > 0.0f) {
        if (sync) {
            PWIDTH_LOOP_PM_SYNC(
                output[i] = gb(gtri(phase, width));
            )
        } else {
            PWIDTH_LOOP_PM(
                output[i] = gb(gtri(phase, width));
            )
        }
    } else if (sync) {
        // bandlimited
        PWIDTH_LOOP_SYNC(
            float s = input_sync[i];
            float mod = 0.0f;
            if (s >= 0.0f) { // start
                mod = gtri(phase_ + inc - phase, width) * polyblep(s);
            } else if (s > -1.0f) { // end
                mod = gtri(phase + sync * inc, width) * polyblep(s);
            }
            output[i] = gb(gtri(phase, width) - mod);
        )
    } else {
        PWIDTH_LOOP(
            output[i] = gb(gtri(phase, width));
        )
    }
}

// polyblep
void Virtual::el_pulse(float* output, float* out_sync, int samples) {
    if (pm > 0.0f) {
        if (sync) {
            PWIDTH_LOOP_PM_SYNC(
                output[i] = phase < width ? -1.0f : 1.0f;
            )
        } else {
            PWIDTH_LOOP_PM(
                output[i] = phase < width ? -1.0f : 1.0f;
            )
        }
    } else if (sync) {
        // FIXME
        PWIDTH_LOOP_SYNC(
            float mod = pulse_sync(phase, phase_, width, inc, input_sync[i]);
            output[i] = (phase < width ? -1.0f : 1.0f) - 2.0f * mod;
        )
    } else {
        // bandlimited
        PWIDTH_LOOP(
            float mod = pulse_polyblep(phase, width, inc);
            output[i] = (phase < width ? -1.0f : 1.0f) - 2.0f * mod;
        )
    }
}

static float pulse_saw(float phase, float width) {
    if (phase < width) {
        return phase / width;
    } else {
        return (width - phase) / (1.0f - width);
    }
}

// polyblep
void Virtual::el_pulse_saw(float* output, float* out_sync, int samples) {
    if (pm > 0.0f) {
        if (sync) {
            PWIDTH_LOOP_PM_SYNC(
                output[i] = pulse_saw(phase, width);
            )
        } else {
            PWIDTH_LOOP_PM(
                output[i] = pulse_saw(phase, width);
            )
        }
    } else if (sync) {
        // bandlimited
        PWIDTH_LOOP_SYNC(
            float s = input_sync[i];
            float p2 = pulse_saw(phase, width);
            float mod = 0.0f;
            if (s >= 0.0f) { // sync start
                mod = pulse_saw(phase_, width) * polyblep(s);
            } else if (phase < inc) { // start
                mod = -polyblep(phase / inc);
            } else if (phase > (1.0f - inc)) { // end
                mod = -polyblep( (phase - 1.0f) / inc);
            } else if (phase < width && phase > (width - inc)) { // mid end
                mod = polyblep( (phase - width) / inc);
            } else if (phase > width && phase < (width + inc)) { // mid start
                mod = polyblep((phase - width) / inc);
            } else if (s > -1.0f) { // sync end
                mod = pulse_saw(phase, width) * polyblep(s);
            }
            output[i] = p2 - mod;
        )
    } else {
        // bandlimited
        PWIDTH_LOOP(
            float p2 = pulse_saw(phase, width);
            float mod = 0.0f;
            if (phase < inc) { // start
                mod = -polyblep(phase / inc);
            } else if (phase > (1.0f - inc)) { // end
                mod = -polyblep( (phase - 1.0f) / inc);
            } else if (phase < width && phase > (width - inc)) { // mid end
                mod = polyblep( (phase - width) / inc);
            } else if (phase > width && phase < (width + inc)) { // mid start
                mod = polyblep((phase - width) / inc);
            }
            output[i] = p2 - mod;
        )
    }
}

// polyblep
void Virtual::el_slope(float* output, float* out_sync, int samples) {
    if (pm > 0.0f) {
        if (sync) {
            PWIDTH_LOOP_PM_SYNC(
                output[i] = gb(gvslope(phase, width));
            )
        } else {
            PWIDTH_LOOP_PM(
                output[i] = gb(gvslope(phase, width));
            )
        }
    } else if (sync) {
        // bandlimited
        PWIDTH_LOOP_SYNC(
            float s = input_sync[i];
            float p2 = gvslope(phase, width);
            float mod = 0.0f;
            if (s >= 0.0f) { // sync start
                mod = gvslope(phase_, width) * polyblep(s);
            } else if (s > -1.0f) { // sync end
                mod = gvslope(phase, width) * polyblep(s);
            } else if (phase < inc) { // start
                mod = polyblep(phase / inc);
            } else if (phase > (1.0f - inc)) { // end
                mod = polyblep( (phase - 1.0f) / inc);
            } else if (phase < width && phase > (width - inc)) { // mid end
                mod = width * polyblep( (phase - width) / inc);
            } else if (phase > width && phase < (width + inc)) { // mid start
                mod = width * polyblep((phase - width) / inc);
            }
            output[i] = gb(p2 - mod);
        )
    } else {
        // bandlimited
        PWIDTH_LOOP(
            float p2 = gvslope(phase, width);
            float mod = 0.0f;
            if (phase < inc) { // start
                mod = polyblep(phase / inc);
            } else if (phase > (1.0f - inc)) { // end
                mod = polyblep( (phase - 1.0f) / inc);
            } else if (phase < width && phase > (width - inc)) { // mid end
                mod = width * polyblep( (phase - width) / inc);
            } else if (phase > width && phase < (width + inc)) { // mid start
                mod = width * polyblep((phase - width) / inc);
            }
            output[i] = gb(p2 - mod);
        )
    }
}

void Virtual::el_alpha1(float* output, float* out_sync, int samples) {
    // pulse
    float f = freq;
    float p = phase;
    float p_ = phase_;
    freq = 2.0f * freq;
    phase = fmod(2.0f * phase, 1.0);
    phase_ = fmod(2.0f * phase_, 1.0);
    el_pulse(output, out_sync, samples);

    // saw
    phase = p;
    phase_ = p_;
    freq = f;
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                output[i] = phase * (output[i] + 1.0) - 1.0f;
            )
        } else {
            PHASE_LOOP_PM(
                output[i] = phase * (output[i] + 1.0) - 1.0f;
            )
        }
    } else if (sync) {
        // bandlimited
        PHASE_LOOP_SYNC(
            float mod = saw_sync(phase, phase_, inc, input_sync[i]);
            output[i] = (phase - mod) * (output[i] + 1.0) - 1.0f;
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            float mod = saw_polyblep(phase, inc);
            output[i] = (phase - mod) * (output[i] + 1.0) - 1.0f;
        )
    }

}

void Virtual::el_alpha2(float* output, float* out_sync, int samples) {
    // pulse
    float f = freq;
    float p = phase;
    float p_ = phase_;
    freq = 4.0f * freq;
    phase = fmod(4.0f * phase, 1.0);
    phase_ = fmod(4.0f * phase_, 1.0);
    el_pulse(output, out_sync, samples);

    // saw
    phase = p;
    phase_ = p_;
    freq = f;
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                output[i] = phase * (output[i] + 1.0) - 1.0f;
            )
        } else {
            PHASE_LOOP_PM(
                output[i] = phase * (output[i] + 1.0) - 1.0f;
            )
        }
    } else if (sync) {
        // bandlimited
        PHASE_LOOP_SYNC(
            float mod = saw_sync(phase, phase_, inc, input_sync[i]);
            output[i] = (phase - mod) * (output[i] + 1.0) - 1.0f;
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            float mod = saw_polyblep(phase, inc);
            output[i] = (phase - mod) * (output[i] + 1.0) - 1.0f;
        )
    }
}

// copied from lmms triple oscillator
static float lmms_exp(float phase) {
    if (phase > 0.5f) {
        return -1.0 + 8.0 * (1.0 - phase) * (1.0 - phase);
    } else {
        return -1.0 + 8.0 * phase * phase;
    }
}

void Virtual::el_exp(float* output, float* out_sync, int samples) {
    if (pm > 0.0f) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                output[i] = lmms_exp(phase);
            )
        } else {
            PHASE_LOOP_PM(
                output[i] = lmms_exp(phase);
            )
        }
    } else if (sync) {
        PHASE_LOOP_SYNC(
            // TODO
            output[i] = lmms_exp(phase);
        )
    } else {
        PHASE_LOOP(
            output[i] = lmms_exp(phase);
        )
    }
}

// FM

void Virtual::fm1(float* output, float* out_sync, int samples) {
    PHASE_LOOP_BOTH(
        output[i] = SIN(phase);
    )
}

void Virtual::fm2(float* output, float* out_sync, int samples) {
    PHASE_LOOP_BOTH(
        output[i] = SIN(0.5f * phase);
    )
}

// bandlimited
void Virtual::fm3(float* output, float* out_sync, int samples) {
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                float y = SIN(phase);
                output[i] = (phase > 0.25 && phase < 0.75) ? -y : y;
            )
        } else {
            PHASE_LOOP_PM(
                float y = SIN(phase);
                output[i] = (phase > 0.25 && phase < 0.75) ? -y : y;
            )
        }
    } else if (sync) {
        PHASE_LOOP_SYNC(
            float y = SIN(phase);
            output[i] = (phase > 0.25 && phase < 0.75) ? -y : y;
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            float y = SIN(phase);
            if (phase < (0.25 - inc)) {
                // do nothing
            } else if (phase < 0.25) {
                // fade out
                y *= (0.25 - phase) / inc;
            } else if (phase < (0.25 + inc)) {
                // fade in (inverted)
                y *= (0.25 - phase) / inc;
            } else if (phase < (0.75 - inc)) {
                y *= -1.0;
            } else if (phase < 0.75) {
                // fade out
                y *= (phase - 0.75) / inc;
            } else if (phase < (0.75 + inc)) {
                // fade in
                y *= (phase - 0.75) / inc;
            }
            output[i] = y;
        )
    }
}

// bandlimited
void Virtual::fm4(float* output, float* out_sync, int samples) {
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                output[i] = phase < 0.5f ? SIN(2.0f * phase) : 0.0f;
            )
        } else {
            PHASE_LOOP_PM(
                output[i] = phase < 0.5f ? SIN(2.0f * phase) : 0.0f;
            )
        }
    } else if (sync) {
        PHASE_LOOP_SYNC(
            output[i] = phase < 0.5f ? SIN(2.0f * phase) : 0.0f;
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            if (phase < 0.5) {
                output[i] = SIN(2.0f * phase);
            } else if (phase < (0.5 + inc)) {
                // fade out
                output[i] = SIN(2.0f * phase - 1.0f) * ((0.5 + inc) - phase) / inc;
            } else {
                output[i] = 0.0;
            }
        )
    }

}

// bandlimited
void Virtual::fm5(float* output, float* out_sync, int samples) {
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                output[i] = phase < 0.5f ? SIN(phase) : 0.0f;
            )
        } else {
            PHASE_LOOP_PM(
                output[i] = phase < 0.5f ? SIN(phase) : 0.0f;
            )
        }
    } else if (sync) {
        PHASE_LOOP_SYNC(
            output[i] = phase < 0.5f ? SIN(phase) : 0.0f;
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            if (phase < 0.5) {
                output[i] = SIN(phase);
            } else if (phase < (0.5 + inc)) {
                // fade out
                output[i] = SIN(phase) * ((0.5 + inc) - phase) / inc;
            } else {
                output[i] = 0.0;
            }
        )
    }

}

static float bandlimit_fm678(float y, float phase, float inc) {
    if (phase < 0.25f) {
        // do nothing;
    } else if (phase < (0.25 + inc)) {
        // fade out
        y *= ((0.25 + inc) - phase) / inc;
    } else if (phase < (0.5 - inc)) {
        y = 0.0f;
    } else if (phase < 0.5) {
        // fade in
        y *= (phase - (0.5 - inc)) / inc;
    } else if (phase < 0.75f) {
        // do nothing
    } else if (phase < (0.75f + inc)) {
        // fade out
        y *= ((0.75 + inc) - phase) / inc;
    } else {
        y = 0.0f;
    }
    return y;
}

// bandlimited
void Virtual::fm6(float* output, float* out_sync, int samples) {
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                if (phase < 0.25f) {
                     output[i] = SIN(2.0 * phase);
                 } else if (phase > 0.5f && phase < 0.75f) {
                     output[i] = SIN(2.0 * (phase - 0.25));
                 } else {
                     output[i] = 0.0f;
                 }
            )
        } else {
            PHASE_LOOP_PM(
                if (phase < 0.25f) {
                     output[i] = SIN(2.0 * phase);
                 } else if (phase > 0.5f && phase < 0.75f) {
                     output[i] = SIN(2.0 * (phase - 0.25));
                 } else {
                     output[i] = 0.0f;
                 }
            )
        }

    } else if (sync) {
        PHASE_LOOP_SYNC(
            if (phase < 0.25f) {
                 output[i] = SIN(2.0 * phase);
             } else if (phase > 0.5f && phase < 0.75f) {
                 output[i] = SIN(2.0 * (phase - 0.25));
             } else {
                 output[i] = 0.0f;
             }
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            float y = 0.0f;
            if (phase < 0.25f) {
                y = SIN(2.0 * phase);
            } else if (phase > 0.5f && phase < 0.75f) {
                y = SIN(2.0 * (phase - 0.25));
            }
            output[i] = bandlimit_fm678(y, phase, inc);
        )
    }

}

// bandlimited
void Virtual::fm7(float* output, float* out_sync, int samples) {
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                if (phase < 0.25 || phase > 0.5 && phase < 0.75) {
                    output[i] = SIN(phase);
                } else {
                    output[i] = 0.0f;
                }
            )
        } else {
            PHASE_LOOP_PM(
                if (phase < 0.25 || phase > 0.5 && phase < 0.75) {
                    output[i] = SIN(phase);
                } else {
                    output[i] = 0.0f;
                }
            )
        }

    } else if (sync) {
        PHASE_LOOP_SYNC(
            if (phase < 0.25 || phase > 0.5 && phase < 0.75) {
                output[i] = SIN(phase);
            } else {
                output[i] = 0.0f;
            }
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            const float y = SIN(phase);
            output[i] = bandlimit_fm678(y, phase, inc);
        )
    }

}

// bandlimited
void Virtual::fm8(float* output, float* out_sync, int samples) {
    if (pm > 0.0) {
        if (sync) {
            PHASE_LOOP_PM_SYNC(
                if (phase < 0.25 || phase > 0.5 && phase < 0.75) {
                    output[i] = SIN(fmod(phase, 0.25f));
                } else {
                    output[i] = 0.0f;
                }
            )
        } else {
            PHASE_LOOP_PM(
                if (phase < 0.25 || phase > 0.5 && phase < 0.75) {
                    output[i] = SIN(fmod(phase, 0.25f));
                } else {
                    output[i] = 0.0f;
                }
            )
        }

    } else if (sync) {
        PHASE_LOOP_SYNC(
            if (phase < 0.25 || phase > 0.5 && phase < 0.75) {
                output[i] = SIN(fmod(phase, 0.25f));
            } else {
                output[i] = 0.0f;
            }
        )
    } else {
        // bandlimited
        PHASE_LOOP(
            const float y = fabs(SIN(fmod(phase, 0.5f)));
            output[i] = bandlimit_fm678(y, phase, inc);
        )
    }

}

void Virtual::process(float* output, float* out_sync, int samples) {
    switch (type) {
    // va
    CASE(VA_SAW, va_saw)
    CASE(VA_TRI_SAW, va_tri_saw)
    CASE(VA_PULSE, va_pulse)
    // pd
    CASE(PD_SAW, pd_saw)
    CASE(PD_SQUARE, pd_square)
    CASE(PD_PULSE, pd_pulse)
    CASE(PD_DOUBLE_SINE, pd_double_sine)
    CASE(PD_SAW_PULSE, pd_saw_pulse)
    CASE(PD_RES1, pd_res1)
    CASE(PD_RES2, pd_res2)
    CASE(PD_RES3, pd_res3)
    CASE(PD_HALF_SINE, pd_half_sine)
    // el
    CASE(EL_SAW, el_saw)
    CASE(EL_DOUBLE_SAW, el_double_saw)
    CASE(EL_TRI, el_tri)
    CASE(EL_PULSE, el_pulse)
    CASE(EL_PULSE_SAW, el_pulse_saw)
    CASE(EL_SLOPE, el_slope)
    CASE(EL_ALPHA1, el_alpha1)
    CASE(EL_ALPHA2, el_alpha2)
    CASE(EL_EXP, el_exp)
    // fm
    CASE(FM1, fm1)
    CASE(FM2, fm2)
    CASE(FM3, fm3)
    CASE(FM4, fm4)
    CASE(FM5, fm5)
    CASE(FM6, fm6)
    CASE(FM7, fm7)
    CASE(FM8, fm8)
    }
}

// AS

// TODO add pm support
void AS::saw(float* output, float* out_sync, int samples) {
    float inc = freq / sample_rate;
    float p = phase;
    int max = std::min(20.0f * wt, sample_rate / 2.0f / freq);

    // first
    for (uint i = 0; i < samples; i++) {
        INC_PHASE()
        output[i] = SIN(phase);
    }
    float end = phase;

    // others
    for (uint j = 2; j < max; j++) {
        phase = fmod(j * p, 1.0f);
        float inc2 = j * inc;
        float scale = 1.0/j;
        for (uint i = 0; i < samples; i++) {
            phase += inc2;
            if (phase >= 1.0f) phase -= 1.0f;
            output[i] += scale * SIN(phase);
       }
    }

    // normalize
    for (uint i = 0; i < samples; i++) {
        output[i] *= -2.0f/M_PI;
    }

    phase = end;
}

// TODO add pm support
void AS::square(float* output, float* out_sync, int samples) {
    float inc = freq / sample_rate;
    float p = phase;
    int max = std::min(40.0f * wt, sample_rate / 2.0f / freq);

    // first
    for (uint i = 0; i < samples; i++) {
        INC_PHASE()
        output[i] = SIN(phase);
    }
    float end = phase;

    // others
    for (uint j = 3; j < max; j += 2) {
        phase = fmod(j * p, 1.0f);
        float inc2 = j * inc;
        float scale = 1.0/j;
        for (uint i = 0; i < samples; i++) {
            phase += inc2;
            if (phase >= 1.0f) phase -= 1.0f;
            output[i] += scale * SIN(phase);
       }
    }

    // normalize
    for (uint i = 0; i < samples; i++) {
        output[i] *= -4.0/M_PI;
    }

    phase = end;
}

// TODO add pm support
void AS::triangle(float* output, float* out_sync, int samples) {
    float inc = freq / sample_rate;
    float p = phase;
    int max = std::min(40.0f * wt, sample_rate / 2.0f / freq);

    // first
    for (uint i = 0; i < samples; i++) {
        INC_PHASE()
        output[i] = SIN(phase);
    }
    float end = phase;

    // others
    float inv = -1.0f;
    for (uint j = 3; j < max; j += 2) {
        phase = fmod(j * p, 1.0f);
        float inc2 = j * inc;
        float scale = inv * 1.0/(j*j);
        for (uint i = 0; i < samples; i++) {
            phase += inc2;
            if (phase >= 1.0f) phase -= 1.0f;
            output[i] += scale * SIN(phase);
       }
       inv *= -1.0f;
    }

    // normalize
    for (uint i = 0; i < samples; i++) {
        output[i] *= -8.0/(M_PI * M_PI);
    }

    phase = end;
}

void AS::process(float* output, float* out_sync, int samples) {
    switch (type) {
    CASE(SAW, saw)
    CASE(SQUARE, square)
    CASE(TRIANGLE, triangle)
    }
}

// SuperWave

void SuperWave::clear() {
    Oscillator::clear();
    for (uint i = 0; i < 7; i++) {
        phases[i] = start * rand() / (RAND_MAX + 1.0f);
    }
}

void SuperWave::reset() {
    Oscillator::reset();
    for (uint i = 0; i < 7; i++) {
        phases[i] = start * rand() / (RAND_MAX + 1.0f);
    }
}

void SuperWave::saw(float* output, float* out_sync, int samples) {
    float inc = freq / sample_rate;
    float incs[7];

    float off = -0.2;
    for (uint j = 0; j < 7; j++) {
        incs[j] = inc * std::pow(SEMITONE, wf * off);
        off += 0.2/3.0;
    }

    for (uint i = 0; i < samples; i++) {
        float out = 0;
        for (uint j = 0; j < 7; j++) {
            phases[j] += incs[j];
            if (phases[j] >= 1.0f) phases[j] -= 1.0f;
            out += gb(phases[j]);
        }
        output[i] = out / 7.0f;
    }
}

void SuperWave::square(float* output, float* out_sync, int samples) {
    float inc = freq / sample_rate;
    float incs[7];

    float off = -0.2;
    for (uint j = 0; j < 7; j++) {
        incs[j] = inc * std::pow(SEMITONE, wf * off);
        off += 0.2/3.0;
    }

    for (uint i = 0; i < samples; i++) {
        float out = 0;
        for (uint j = 0; j < 7; j++) {
            phases[j] += incs[j];
            if (phases[j] >= 1.0f) phases[j] -= 1.0f;
            out += phases[j] < 0.5 ? -1.0 : 1.0;
        }
        output[i] = out / 7.0f;
    }
}

void SuperWave::process(float* output, float* out_sync, int samples) {
    switch (type) {
    CASE(SAW, saw)
    CASE(SQUARE, square)
    }
}


// Noise

void Noise::process(float* output, float* out_sync, int samples) {
    for (uint i = 0; i < samples; i++) {
        output[i] =  (2.0f * rand() / (RAND_MAX + 1.0f) - 1.0f);
        out_sync[i] = -2.0;
    }

    if (type == PINK) {
        // Paul Kellet's pink noise
        // http://musicdsp.org/files/pink.txt
        for (uint i = 0; i < samples; i++) {
            float white = output[i];
            b0 = 0.99765 * b0 + white * 0.0990460;
            b1 = 0.96300 * b1 + white * 0.2965164;
            b2 = 0.57000 * b2 + white * 1.0526913;
            output[i] = b0 + b1 + b2 + white * 0.1848;
        }
    } else if (type == LP) {
        filter.setType(0);
        filter.setCoefficients(freq, wf);
        filter.process(output, output, samples);
    } else if (type == BP) {
        filter.setType(2);
        filter.setCoefficients(freq, wf);
        filter.process(output, output, samples);
    }
}

}
