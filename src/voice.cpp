/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * uses code from https://github.com/rekado/lv2-mdaPiano
 *                https://github.com/smbolton/whysynth
 */

#include <stdio.h>
#include "voice.h"

#define SEMITONE 1.05946f

namespace rogue {

// TODO use table for this
static float midi2f(unsigned char data) {
    return 0.007874f * (float)(data);
}

static float midi2hz(float key) {
    return 8.177445f * std::pow(SEMITONE, key);
}

rogueVoice::rogueVoice(double rate, SynthData* d) {
    data = d;
    sample_rate = rate;

    left = p(p_left);
    right = p(p_right);

    // init elements
    for (int i = 0; i < NOSC; i++) oscs[i] = Osc();
    for (int i = 0; i < NDCF; i++) filters[i] = Filter();
    for (int i = 0; i < NLFO; i++) lfos[i] = LFO();
    for (int i = 0; i < NENV; i++) envs[i] = Env();

    // set sample rate
    for (int i = 0; i < NOSC; i++) oscs[i].setSamplerate(rate);
    for (int i = 0; i < NDCF; i++) filters[i].setSamplerate(rate);

    // set buffers
    buffers[0] = bus_a;
    buffers[1] = bus_b;
    buffers[2] = filters[0].buffer;
    buffers[3] = filters[1].buffer;
}

void rogueVoice::on(unsigned char key, unsigned char velocity) {
    std::cout << "on " << int(key) << " " << int(velocity) << std::endl;

    if (velocity == 0) {
        off(0);
        return;
    }

    // store key that turned this voice on (used in 'get_key')
    m_key = key;
    m_velocity = velocity;
    mod[M_KEY] = midi2f(key);
    mod[M_VEL] = midi2f(velocity);

    // config
    for (int i = 0; i < NLFO; i++) configLFO(i);
    for (int i = 0; i < NENV; i++) configEnv(i);
    for (int i = 0; i < NOSC; i++) configOsc(i);
    for (int i = 0; i < NDCF; i++) configFilter(i);

    // trigger on
    for (int i = 0; i < NLFO; i++) lfos[i].on();
    for (int i = 0; i < NENV; i++) envs[i].on();
    for (int i = 0; i < NOSC; i++) {
        if (!data->oscs[i].free) oscs[i].reset();
        oscs[i].prev_level = 0.0f;
    }
    for (int i = 0; i < NDCF; i++) {
        filters[i].prev_level = 0.0f;
    }

    in_sustain = false;
}

void rogueVoice::off(unsigned char velocity) {
    std::cout << "off " << int(m_key) << " " << int(velocity) << std::endl;
    // trigger off
    for (int i = 0; i < NLFO; i++) lfos[i].off();
    for (int i = 0; i < NENV; i++) envs[i].off();

    //Mark the voice to be turned off later. It may not be set to
    //INVALID_KEY yet, because the release sound still needs to be
    //rendered.  m_key is finally set to INVALID_KEY by 'render' when
    //env < SILENCE
    in_sustain = true;
}

static float amp_modulate(float amt, float val) {
    if (amt > 0) {
        return 1.0f - amt + amt * val;
    } else {
        return 1.0f + amt * val;
    }
}

// TODO optimize
float rogueVoice::modulate(int target) {
    float v = 1.0f;
    for (int i = 0; i < NMOD; i++) {
        if (data->mods[i].target == target) {
            ModulationData& modData = data->mods[i];
            v *= amp_modulate(modData.amount, mod[modData.src]);
        }
    }
    return v;
}

// TODO optimize
float rogueVoice::pitch_modulate(int target) {
    float v = 0.0f;
    for (int i = 0; i < NMOD; i++) {
        if (data->mods[i].target == target) {
            ModulationData& modData = data->mods[i];
            v += 12.0f * modData.amount * mod[modData.src];
        }
    }
    return v;
}

void rogueVoice::configLFO(int i) {
    LFOData& lfoData = data->lfos[i];
    LFO& lfo = lfos[i];

    float f = lfoData.freq;
    // key to f
    f *= modulate(M_LFO1_S + 2 * i);

    // NOTE: lfos can't modulate each other
    lfo.lfo.setType(lfoData.type);
    lfo.lfo.setFreq(f);
    lfo.lfo.setSymmetry(lfoData.symmetry);
    // TODO humanize
    // TODO reset type
}

void rogueVoice::runLFO(int i, uint32_t from, uint32_t to) {
    LFOData& lfoData = data->lfos[i];
    LFO& lfo = lfos[i];
    float v = 0.0f;
    if (lfoData.on) {
        v = lfo.lfo.tick(to - from);
        if (lfoData.inv) {
            v *= -1.0f;
        }

        // amp modulation
        v *= modulate(M_LFO1_AMP + 2 * i);
    }
    // update mod values
    mod[M_LFO1_BI + 2*i] = v;
    mod[M_LFO1_UN + 2*i] = 0.5 * v + 0.5;

    lfo.last = lfo.current;
    lfo.current = 0.0f;
}

void rogueVoice::configEnv(int i) {
    EnvData& envData = data->envs[i];
    Env& env = envs[i];

    // NOTE: envelopes can't modulate each other
    float f = 1.0f / modulate(M_ENV1_S + 2 * i); // TODO use different modulation algorithm
    float a = envData.attack / f;
    float h = envData.hold / f;
    float d = envData.decay / f;
    float s = envData.sustain;
    float r = envData.release / f;
    // TODO pre-delay
    // TODO retrigger
    env.env.setAHDSR(a, h, d, s, r);
}

void rogueVoice::runEnv(int i, uint32_t from, uint32_t to) {
    EnvData& envData = data->envs[i];
    Env& env = envs[i];
    float v = 0.0f;
    if (envData.on) {
        v = env.env.tick(to - from);
        // amp modulation
        v *= modulate(M_ENV1_AMP + 2 * i);
    }
    // update mod values
    mod[M_ENV1 + i] = v;

    env.last = env.current;
    env.current = v;
}

void rogueVoice::configOsc(int i) {
    OscData& oscData = data->oscs[i];
    Osc& osc = oscs[i];
    // TODO
}

void rogueVoice::runOsc(int i, uint32_t from, uint32_t to) {
    OscData& oscData = data->oscs[i];
    Osc& osc = oscs[i];
    if (oscData.on) {
        // pitch modulation
        float f = 440.0;
        float pmod = pitch_modulate(M_OSC1_P + 3 * i);
        if (oscData.tracking) {
            f = midi2hz(float(m_key) + oscData.coarse + oscData.fine + pmod);
        } else if (pmod > 0.0f) {
            f = midi2hz(69.0f + pmod);
        }
        f *= oscData.ratio;

        // TODO mod modulation

        // process
        osc.osc.setType(oscData.type);
        osc.osc.setFreq(f);
        osc.osc.setParams(oscData.param1, oscData.param2);
        osc.osc.process(osc.buffer + from, to - from);

        // amp modulation
        float v = oscData.level;
        if (oscData.inv) {
            v *= -1.0f;
        }
        if (oscData.vel_to_vol > 0.0f) {
            v *= 1.0 - oscData.vel_to_vol + oscData.vel_to_vol * midi2f(m_velocity);
        }

        v *= modulate(M_OSC1_AMP + 3 * i);
        float step = (v - osc.prev_level) / (to - from);
        float l = osc.prev_level;
        for (int i = from; i < to; i++) {
            osc.buffer[i] *= l;
            l += step;
        }
        osc.prev_level = v;

        // copy to buffers
        for (int i = from; i < to; i++) {
            bus_a[i] += oscData.level_a * osc.buffer[i];
            bus_b[i] += oscData.level_b * osc.buffer[i];
        }
    }
}

void rogueVoice::configFilter(int i) {
    FilterData& filterData = data->filters[i];
    Filter& filter = filters[i];

    float f = 1.0;
    // key to f
    if (filterData.key_to_f != 0.0f) {
        f *= std::pow(SEMITONE, filterData.key_to_f * float(m_key - 69));
    }
    // vel to f
    if (filterData.vel_to_f != 0.0f) {
        f *= std::pow(SEMITONE, filterData.vel_to_f * float(m_velocity - 64));
    }
    filter.key_vel_to_f = f;
}

void rogueVoice::runFilter(int i, uint32_t from, uint32_t to) {
    FilterData& filterData = data->filters[i];
    Filter& filter = filters[i];
    if (filterData.on) {
        int type = filterData.type;
        float f = filterData.freq * filter.key_vel_to_f;

        // freq modulation
        f *= modulate(M_DCF1_F + 4 * i); // TODO use different modulation algorithm here

        // res modulation
        float q = filterData.q;
        q *= modulate(M_DCF1_Q + 4 * i);

        // process
        float* source = buffers[filterData.source];
        if (type < 8) {
            filter.moog.setType(type);
            filter.moog.setCoefficients(f, q);
            filter.moog.process(source + from, filter.buffer + from, to - from);
        } else {
            filter.svf.setType(type - 8);
            filter.svf.setCoefficients(f, q);
            filter.svf.process(source + from, filter.buffer + from, to - from);
        }

        // amp modulation
        float v = filterData.level;
        v *= modulate(M_DCF1_AMP + 4 * i);
        float step = (v - filter.prev_level) / (to - from);
        float l = filter.prev_level;
        for (int i = from; i < to; i++) {
            filter.buffer[i] *= l;
            l += step;
        }
        filter.prev_level = v;
    }
}

void rogueVoice::render(uint32_t from, uint32_t to) {
    uint32_t from_ = from % BUFFER_SIZE;
    uint32_t off = from - from_;
    while (off < to) {
        render(from_, std::min(to - off, uint32_t(BUFFER_SIZE)), off);
        off += BUFFER_SIZE;
        from_ = 0;
    }
}

void rogueVoice::render(uint32_t from, uint32_t to, uint32_t off) {
    if (m_key == lvtk::INVALID_KEY) {
        return;
    }

    // reset buses
    std::memset(bus_a, 0, sizeof(float) * BUFFER_SIZE);
    std::memset(bus_b, 0, sizeof(float) * BUFFER_SIZE);

    // run elements
    for (int i = 0; i < NLFO; i++) runLFO(i, from, to);
    for (int i = 0; i < NENV; i++) runEnv(i, from, to);
    for (int i = 0; i < NOSC; i++) runOsc(i, from, to);
    for (int i = 0; i < NDCF; i++) runFilter(i, from, to);

    // pan config (not interpolated)
    float left_p[] = {1.0f * data->filters[0].pan,
                      1.0f * data->filters[1].pan,
                      data->bus_a_level * (1.0f - data->bus_a_pan),
                      data->bus_b_level * (1.0f - data->bus_b_pan)};
    float right_p[] = {data->filters[0].pan,
                       data->filters[1].pan,
                       data->bus_a_level * data->bus_a_pan,
                       data->bus_b_level * data->bus_b_pan};

    // TODO filter1 pan modulation
    // TODO filter2 pan modulation
    // TODO bus a pan modulation
    // TODO bus b pan modulation

    // amp modulation
    float e_from = envs[0].last;
    float e_step = (envs[0].current - e_from) / float(to - from);

    // copy buffers
    for (int i = from; i < to; i++) {
        for (int j = 0; j < 4; j++) {
            float sample = buffers[j][i];
            left[off + i]  += left_p[j] * sample;
            right[off + i] += right_p[j] * sample;
        }
        e_from += e_step;
    }

    // close voice, if too silent
    if (envs[0].current < SILENCE) {
        reset();
    }
}

void rogueVoice::reset() {
    volume = 1.0f;
    m_key = lvtk::INVALID_KEY;
    in_sustain = false;
    std::memset(mod, 0, sizeof(float) * NMOD);
}


}
