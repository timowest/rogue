/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * uses code from https://github.com/rekado/lv2-mdaPiano
 *                https://github.com/smbolton/whysynth
 */

#include "voice.h"

#define SEMITONE 1.05946f

namespace rogue {

static float midi2f_values[256];

static float midi2f(unsigned char data) {
    return midi2f_values[data];
}

static float midi2hz(float key) {
    return 8.177445f * std::pow(SEMITONE, key);
}

static uint init_values() {
    for (uint i = 0; i < 256; i++) {
        midi2f_values[i] = 0.007874f * float(i);
    }
}

static uint _ = init_values();

static float limit(float v, float min, float max) {
    if (v < min) {
        return min;
    } else if (v > max) {
        return max;
    } else {
        return v;
    }
}

rogueVoice::rogueVoice(double rate, SynthData* d, float* l, float* r) {
    data = d;
    sample_rate = d->oversample * rate;
    half_sample_rate = 0.5f * rate;
    left = l;
    right = r;

    // init elements
    for (uint i = 0; i < NOSC; i++) oscs[i] = Osc();
    for (uint i = 0; i < NDCF; i++) filters[i] = Filter();
    for (uint i = 0; i < NLFO; i++) lfos[i] = LFO();
    for (uint i = 0; i < NENV; i++) envs[i] = Env();

    // set sample rate
    for (uint i = 0; i < NOSC; i++) oscs[i].setSamplerate(sample_rate);
    for (uint i = 0; i < NDCF; i++) filters[i].setSamplerate(sample_rate);
    for (uint i = 0; i < NLFO; i++) lfos[0].setSamplerate(sample_rate);

    // set buffers
    buffers[0] = bus_a;
    buffers[1] = bus_b;
    buffers[2] = filters[0].buffer;
    buffers[3] = filters[1].buffer;
}

void rogueVoice::on(unsigned char key, unsigned char velocity) {
    //std::cout << "on " << int(key) << " " << int(velocity) << std::endl;

    if (velocity == 0) {
        off(0);
        return;
    }

    unsigned char old_key = m_key;

    // store key that turned this voice on (used in 'get_key')
    m_key = key;
    m_velocity = velocity;
    this->velocity = velocity;
    mod[M_KEY] = midi2f(key);
    mod[M_VEL] = midi2f(velocity);

    // glide
    if (data->playmode == LEGATO && old_key != lvtk::INVALID_KEY) {
        glide_step = (float(key) - this->key) / (data->glide_time * sample_rate);
        glide_target = float(key);
    } else {
        glide_step = 0.0;
        this->key = glide_target = key;
    }

    // config
    for (uint i = 0; i < NLFO; i++) configLFO(i);
    for (uint i = 0; i < NENV; i++) configEnv(i);
    for (uint i = 0; i < NOSC; i++) configOsc(i);
    for (uint i = 0; i < NDCF; i++) configFilter(i);

    // trigger on
    for (uint i = 0; i < NLFO; i++) lfos[i].on();
    for (uint i = 0; i < NENV; i++) envs[i].on();
    if (old_key == lvtk::INVALID_KEY || data->playmode == POLY) {
        for (uint i = 0; i < NOSC; i++) {
            if (!data->oscs[i].free) oscs[i].resetPhase();
        }
    }

    in_sustain = false;
}

void rogueVoice::off(unsigned char velocity) {
    //std::cout << "off " << int(m_key) << " " << int(velocity) << std::endl;

    // trigger off
    for (uint i = 0; i < NLFO; i++) lfos[i].off();
    for (uint i = 0; i < NENV; i++) envs[i].off();

    //Mark the voice to be turned off later. It may not be set to
    //INVALID_KEY yet, because the release sound still needs to be
    //rendered.  m_key is finally set to INVALID_KEY by 'render' when
    //env < SILENCE
    in_sustain = true;
}

static float multiply_mod(float v, float amt, float mval) {
    if (amt > 0) {
        return v * (1.0f - amt + amt * mval);
    } else {
        return v * (1.0f + amt * mval);
    }
}

static float add_mod(float v, float amt, float mval) {
    return v + amt * mval;
}

template<class Function>
float rogueVoice::modulate(float init, int target, Function fn) {
    float v = init;
    for (uint i = 0; i < data->mod_count; i++) {
        if (data->mods[i].target == target) {
            ModulationData& modData = data->mods[i];
            v = fn(v, modData.amount, mod[modData.src]);
        }
    }
    return v;
}

void rogueVoice::configLFO(uint i) {
    LFOData& lfoData = data->lfos[i];
    LFO& lfo = lfos[i];

    float f = lfoData.freq;
    // speed modulation
    f *= modulate(1.0f, M_LFO1_S + 2 * i, add_mod);
    if (f < 0.0) f = 0.0;

    // NOTE: lfos can't modulate each other's freq
    lfo.lfo.setType(lfoData.type);
    float start = lfoData.start;
    if (lfoData.reset_type == 1) {
        start = lfoData.phase;
    }
    lfo.lfo.setStart(start);
    lfo.lfo.setFreq(f);
    lfo.lfo.setWidth(lfoData.width);
    // TODO humanize
}

void rogueVoice::runLFO(uint i, uint from, uint to) {
    LFOData& lfoData = data->lfos[i];
    LFO& lfo = lfos[i];
    float v = 0.0f;
    if (lfoData.on) {
        v = lfo.lfo.tick(to - from);
        if (lfoData.inv) {
            v *= -1.0f;
        }

        // amp modulation
        v *= modulate(1.0f, M_LFO1_AMP + 2 * i, multiply_mod);
    }
    // update mod values
    mod[M_LFO1_BI + 2*i] = v;
    mod[M_LFO1_UN + 2*i] = 0.5f * v + 0.5f;

    lfo.last = lfo.current;
    lfo.current = v;
}

void rogueVoice::configEnv(uint i) {
    EnvData& envData = data->envs[i];
    Env& env = envs[i];

    // NOTE: envelopes can't modulate each other's speed
    float f = modulate(1.0f, M_ENV1_S + 2 * i, add_mod);
    if (f < 0.0) f = 0.0;
    float a = envData.attack / f;
    float h = envData.hold / f;
    float d = envData.decay / f;
    float s = envData.sustain;
    float r = envData.release / f;

    env.env.setRetrigger(envData.retrigger);
    env.env.setCurve(envData.curve);
    env.env.setPredelay(envData.pre_delay / f);
    env.env.setAHDSR(std::max(a, 0.001f), h, d, s, r);
}

void rogueVoice::runEnv(uint i, uint from, uint to) {
    EnvData& envData = data->envs[i];
    Env& env = envs[i];
    float v = 0.0f;
    if (envData.on) {
        v = env.env.tick(to - from);
        // amp modulation
        v *= modulate(1.0f, M_ENV1_AMP + 2 * i, multiply_mod);
    }
    // update mod values
    mod[M_ENV1 + i] = v;

    env.last = env.current;
    env.current = v;
}

void rogueVoice::configOsc(uint i) {
    OscData& oscData = data->oscs[i];
    Osc& osc = oscs[i];

    osc.setStart(oscData.start);
}

void rogueVoice::runOsc(uint i, uint from, uint to) {
    OscData& oscData = data->oscs[i];
    Osc& osc = oscs[i];
    if (oscData.on) {
        uint samples = to - from;
        // pitch modulation
        float f = 440.0;
        float pmod = modulate(0.0f, M_OSC1_P + 4 * i, add_mod);
        if (oscData.tracking) {
            f = midi2hz(key + oscData.coarse + oscData.fine + data->pitch_bend + pmod);
        } else if (pmod > 0.0f) {
            f = midi2hz(69.0f + pmod);
        }
        f *= oscData.ratio;

        // pulse width modulation
        float width = oscData.width + modulate(0.0f, M_OSC1_PWM + 4 * i, add_mod);
        width = limit(width, 0, 1);

        // process
        if (i > 0) {
            float* in = oscs[oscData.input].buffer + from;
            float* sync = oscs[oscData.input].sync + from;
            osc.setModulation(oscData.type, in, sync, oscData.pm, oscData.sync);
        }
        osc.process(oscData.type, f, osc.width_prev, width, osc.buffer + from, osc.sync + from, samples);

        // amp modulation
        float v = oscData.level;
        if (oscData.inv) {
            v *= -1.0f;
        }

        v *= modulate(1.0f, M_OSC1_AMP + 4 * i, multiply_mod);
        float step = (v - osc.prev_level) / float(samples);
        float l = osc.prev_level;
        for (uint i = from; i < to; i++) {
            osc.buffer[i] *= l;
            l += step;
        }
        osc.prev_level = v;

        // audio output modulation
        if (oscData.out_mod > 0) {
            float* in = oscs[oscData.input2].buffer;
            switch (oscData.out_mod) {
            case 1: // Add
                for (uint i = from; i < to; i++) {
                    osc.buffer[i] += in[i];
                }
                break;
            case 2: // RM
                for (uint i = from; i < to; i++) {
                    osc.buffer[i] *= in[i];
                }
                break;
            case 3: // AM
                for (uint i = from; i < to; i++) {
                    osc.buffer[i] *= 0.5f * (in[i] + 1.0f);
                }
                break;
            }
        }

        // copy to buffers
        for (uint i = from; i < to; i++) {
            bus_a[i] += oscData.level_a * osc.buffer[i];
            bus_b[i] += oscData.level_b * osc.buffer[i];
        }

        osc.width_prev = width;
    }
}

void rogueVoice::configFilter(uint i) {
    FilterData& filterData = data->filters[i];
    Filter& filter = filters[i];

    float semitones = 0.0f;
    // key to f
    if (filterData.key_to_f != 0.0f) {
        semitones += filterData.key_to_f * (key - 69.0);
    }
    // vel to f
    if (filterData.vel_to_f != 0.0f) {
        semitones += filterData.vel_to_f * (velocity - 64.0);
    }
    if (semitones != 0.0f) {
        filter.key_vel_to_f = std::pow(SEMITONE, semitones);
    } else {
        filter.key_vel_to_f = 1.0;
    }

}

void rogueVoice::runFilter(uint i, uint from, uint to) {
    FilterData& filterData = data->filters[i];
    Filter& filter = filters[i];
    if (filterData.on) {
        uint samples = to - from;
        uint type = filterData.type;
        float f = filterData.freq * filter.key_vel_to_f;

        // freq modulation
        float fmod = modulate(0.0f, M_DCF1_F + 4 * i, add_mod);
        if (fmod != 0.0) {
            f *= std::pow(SEMITONE, 48.0 * fmod);
        }
        f = limit(f, 0, half_sample_rate);

        // res modulation
        float q = filterData.q + modulate(0.0f, M_DCF1_Q + 4 * i, add_mod);
        q = limit(q, 0, 1);

        // process
        float* source = buffers[filterData.source];
        if (type < 6) {
            filter.am.setType(type);
            filter.am.setCoefficients(f, q);
            filter.am.process(source + from, filter.buffer + from, samples);
        } else if (type == 6) {
            filter.moog.setType(0);
            filter.moog.setCoefficients(f, q);
            filter.moog.process(source + from, filter.buffer + from, samples);
        } else if (type < 11) {
            filter.svf.setType(type - 7);
            filter.svf.setCoefficients(f, q);
            filter.svf.process(source + from, filter.buffer + from, samples);
        } else {
            filter.comb.setCoefficients(f, q);
            filter.comb.process(source + from, filter.buffer + from,samples);
        }

        // amp modulation
        float v = modulate(1.0f, M_DCF1_AMP + 4 * i, multiply_mod);
        float step = (v - filter.prev_level) / float(samples);
        float l = filter.prev_level;
        for (uint i = from; i < to; i++) {
            filter.buffer[i] *= l;
            l += step;
        }
        filter.prev_level = v;
    }
}

void rogueVoice::render(uint from, uint to) {
    // oversampling
    from = data->oversample * from;
    to = data->oversample * to;

    uint from_ = from % BUFFER_SIZE;
    uint off = from - from_;
    while (off < to) {
        render(from_, std::min(to - off, uint(BUFFER_SIZE)), off);
        off += BUFFER_SIZE;
        from_ = 0;
    }
}

void rogueVoice::render(uint from, uint to, uint off) {
    if (m_key == lvtk::INVALID_KEY) {
        return;
    }

    if (glide_step != 0.0f) {
        key += (to - from) * glide_step;
        // TODO use counter for this instead
        if ((glide_step > 0.0 && key >= glide_target) ||
            (glide_step < 0.0 && key <= glide_target)) {
            key = glide_target;
            glide_step = 0.0f;
        }
    }

    // reset buses
    std::memset(bus_a, 0, sizeof(float) * BUFFER_SIZE);
    std::memset(bus_b, 0, sizeof(float) * BUFFER_SIZE);

    // run elements
    for (uint i = 0; i < NLFO; i++) runLFO(i, from, to);
    for (uint i = 0; i < NENV; i++) runEnv(i, from, to);
    for (uint i = 0; i < NOSC; i++) runOsc(i, from, to);
    for (uint i = 0; i < NDCF; i++) runFilter(i, from, to);

    // TODO bus a pan modulation
    // TODO bus b pan modulation
    // TODO filter1 pan modulation
    // TODO filter2 pan modulation

    // amp modulation
    const float e_start = envs[0].last;
    const float e_step = (envs[0].current - e_start) / float(to - from);
    float e_vol = e_start;

    // bus a
    if (data->bus_a_level > SILENCE) {
        float l = data->bus_a_level * (1.0f - data->bus_a_pan);
        float r = data->bus_a_level * data->bus_a_pan;
        for (uint i = from; i < to; i++) {
            float sample = e_vol * bus_a[i];
            left[off + i]  += l * sample;
            right[off + i] += r * sample;
            e_vol += e_step;
        }
        e_vol = e_start;
    }

    // bus b
    if (data->bus_b_level > SILENCE) {
        float l = data->bus_b_level * (1.0f - data->bus_b_pan);
        float r = data->bus_b_level * data->bus_b_pan;
        for (uint i = from; i < to; i++) {
            float sample = e_vol * bus_b[i];
            left[off + i]  += l * sample;
            right[off + i] += r * sample;
            e_vol += e_step;
        }
        e_vol = e_start;
    }

    // filter 1
    if (data->filters[0].on && data->filters[0].level > SILENCE) {
        float l = data->filters[0].level * (1.0f - data->filters[0].pan);
        float r = data->filters[0].level * data->filters[0].pan;
        for (uint i = from; i < to; i++) {
            float sample = e_vol * filters[0].buffer[i];
            left[off + i]  += l * sample;
            right[off + i] += r * sample;
            e_vol += e_step;
        }
        e_vol = e_start;
    }

    // filter 2
    if (data->filters[1].on && data->filters[1].level > SILENCE) {
        float l = data->filters[1].level * (1.0f - data->filters[1].pan);
        float r = data->filters[1].level * data->filters[1].pan;
        for (uint i = from; i < to; i++) {
            float sample = e_vol * filters[1].buffer[i];
            left[off + i]  += l * sample;
            right[off + i] += r * sample;
            e_vol += e_step;
        }
        e_vol = e_start;
    }

    // close voice, if too silent
    if (envs[0].current < SILENCE) {
        reset();
    }
}

void rogueVoice::reset() {
    //std::cout << "reset " << int(m_key) << std::endl;

    m_key = lvtk::INVALID_KEY;

    key = m_key;
    in_sustain = false;
    std::memset(mod, 0, sizeof(float) * M_SIZE);

    for (uint i = 0; i < NLFO; i++) lfos[i].reset();
    for (uint i = 0; i < NENV; i++) envs[i].reset();
    for (uint i = 0; i < NOSC; i++) oscs[i].reset();
    for (uint i = 0; i < NDCF; i++) filters[i].reset();
}


}
