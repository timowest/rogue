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

    for (int i = 0; i < NOSC; i++) oscs[i] = Osc();
    for (int i = 0; i < NDCF; i++) filters[i] = Filter();
    for (int i = 0; i < NLFO; i++) lfos[i] = LFO();
    for (int i = 0; i < NENV; i++) envs[i] = Env();

    // set sample rate
    for (int i = 0; i < NOSC; i++) oscs[i].setSamplerate(rate);
    for (int i = 0; i < NDCF; i++) filters[i].setSamplerate(rate);

    mod[M_ON] = 1.0;
}

void rogueVoice::on(unsigned char key, unsigned char velocity) {
    std::cout << "on " << int(key) << " " << int(velocity) << std::endl;
    // store key that turned this voice on (used in 'get_key')
    m_key = key;

    mod[M_KEY] = midi2f(key);
    mod[M_VEL] = midi2f(velocity);

    if (velocity > 0) {
        m_velocity = velocity;
    } else {
        off(0);
    }

    // trigger on
    for (int i = 0; i < NLFO; i++) lfos[i].on();
    for (int i = 0; i < NENV; i++) envs[i].on();
    for (int i = 0; i < NOSC; i++) {
        if (!data->oscs[i].free) oscs[i].reset();
        data->oscs[i].prev_level = 0.0f;
    }
    for (int i = 0; i < NDCF; i++) {
        data->filters[i].prev_level = 0.0f;
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

void rogueVoice::runLFO(int i, uint32_t from, uint32_t to) {
    LFOData& lfoData = data->lfos[i];
    LFO& lfo = lfos[i];
    float v = 0.0f;
    if (lfoData.on) {
        float f = lfoData.freq;
        // key to f
        f *= modulate(M_LFO1_S + 2 * i);

        lfo.lfo.setType(lfoData.type);
        lfo.lfo.setEnv(lfoData.attack, lfoData.decay);
        lfo.lfo.setFreq(lfoData.freq);
        lfo.lfo.setSymmetry(lfoData.symmetry);
        // TODO humanize
        // TODO reset type
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

void rogueVoice::runEnv(int i, uint32_t from, uint32_t to) {
    EnvData& envData = data->envs[i];
    Env& env = envs[i];
    float v = 0.0f;
    if (envData.on) {
        // TODO maybe modulate these parameters only once per note?
        float f = 1.0f / modulate(M_ENV1_S + 2 * i);
        float a = envData.attack / f;
        float h = envData.hold / f;
        float d = envData.decay / f;
        float s = envData.sustain;
        float r = envData.release / f;
        // TODO pre-delay
        // TODO retrigger
        env.env.setAHDSR(a, h, d, s, r);
        v = env.env.tick(to - from);

        // amp modulation
        v *= modulate(M_ENV1_AMP + 2 * i);
    }
    // update mod values
    mod[M_ENV1 + i] = v;

    env.last = env.current;
    env.current = v;
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
        float step = (v - oscData.prev_level) / (to - from);
        float l = oscData.prev_level;
        for (int i = from; i < to; i++) {
            osc.buffer[i] *= l;
            l += step;
        }
        oscData.prev_level = v;

        // copy to buffers
        for (int i = from; i < to; i++) {
            bus_a[i] += oscData.level_a * osc.buffer[i];
            bus_b[i] += oscData.level_b * osc.buffer[i];
        }
    }
}

void rogueVoice::runFilter(int i, uint32_t from, uint32_t to) {
    FilterData& filterData = data->filters[i];
    Filter& filter = filters[i];
    if (filterData.on) {
        int type = filterData.type;
        float f = filterData.freq;
        // key to f
        if (filterData.key_to_f != 0.0f) {
            f *= std::pow(SEMITONE, filterData.key_to_f * float(m_key - 69));
        }
        // vel to f
        if (filterData.vel_to_f != 0.0f) {
            f *= std::pow(SEMITONE, filterData.vel_to_f * float(m_velocity - 64));
        }

        // freq modulation
        f *= modulate(M_DCF1_F + 4 * i);

        // res modulation
        float q = filterData.q;
        q *= modulate(M_DCF1_Q + 4 * i);

        // process
        // TODO put sources into float** variable ?!?
        float* source;
        switch (filterData.source) {
        case 0: source = bus_a; break;
        case 1: source = bus_b; break;
        case 2: source = filters[0].buffer;
        }

        if (type < 8) {
            filter.moog.setType(type);
            filter.moog.setCoefficients(f, filterData.q);
            filter.moog.process(source + from, filter.buffer + from, to - from);
        } else {
            filter.svf.setType(type - 8);
            filter.svf.setCoefficients(f, filterData.q);
            filter.svf.process(source + from, filter.buffer + from, to - from);
        }

        // amp modulation
        float v = filterData.level;
        v *= modulate(M_DCF1_AMP + 4 * i);
        float step = (v - filterData.prev_level) / (to - from);
        float l = filterData.prev_level;
        for (int i = from; i < to; i++) {
            filter.buffer[i] *= l;
            l += step;
        }
        filterData.prev_level = v;
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

    // TODO filter1 pan modulation
    // TODO filter2 pan modulation
    // TODO bus a pan modulation
    // TODO bus b pan modulation

    // copy buses and filters to out
    float f1_l = 1.0 * data->filters[0].pan,
          f1_r = data->filters[0].pan,
          f2_l = 1.0 * data->filters[1].pan,
          f2_r = data->filters[1].pan,
          ba_l = data->bus_a_level * (1.0 - data->bus_a_pan),
          ba_r = data->bus_a_level * data->bus_a_pan,
          bb_l = data->bus_b_level * (1.0 - data->bus_b_pan),
          bb_r = data->bus_b_level * data->bus_b_pan;

    // amp modulation
    float e_from = envs[0].last;
    float e_step = (envs[0].current - e_from) / float(to - from);

    float* left = p(p_left);
    float* right = p(p_right);
    for (int i = from; i < to; i++) {
        left[off + i] += data->volume * e_from *
                  (f1_l * filters[0].buffer[i] +
                   f2_l * filters[1].buffer[i] +
                   ba_l * bus_a[i] +
                   bb_l * bus_b[i]);
        right[off + i] += data->volume * e_from *
                   (f1_r * filters[0].buffer[i] +
                    f2_r * filters[1].buffer[i] +
                    ba_r * bus_a[i] +
                    bb_r * bus_b[i]);
        e_from += e_step;
    }

    if (envs[0].current < SILENCE) {
        m_key = lvtk::INVALID_KEY;
        in_sustain = false;
    }
}

void rogueVoice::reset() {
    volume = 1.0f;
    m_key = lvtk::INVALID_KEY;
    in_sustain = false;
}


}
