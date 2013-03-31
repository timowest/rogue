/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * uses code from https://github.com/rekado/lv2-mdaPiano
 */

#include "voice.h"

namespace rogue {

static float midi_to_f(unsigned char data) {
    return 0.0078f * (float)(data);
}

static float midi2hz(float key) {
    return 8.1758 * std::pow(1.0594, key);
}

rogueVoice::rogueVoice(double rate, SynthData* data) : data(data) {
    sample_rate = rate;
    for (int i = 0; i < NOSC; i++) oscs[i] = rogueOsc();
    for (int i = 0; i < NDCF; i++) filters[i] = rogueFilter();
    for (int i = 0; i < NLFO; i++) lfos[i] = rogueLFO();
    for (int i = 0; i < NENV; i++) envs[i] = rogueEnv();

    // set sample rate
    for (int i = 0; i < NOSC; i++) oscs[i].setSamplerate(rate);
    for (int i = 0; i < NDCF; i++) filters[i].setSamplerate(rate);
}

void rogueVoice::on(unsigned char key, unsigned char velocity) {
    // store key that turned this voice on (used in 'get_key')
    m_key = key;

    if (velocity > 0) {
        m_velocity = midi_to_f(velocity);
    } else {
        off(0);
    }

    // trigger on
    for (int i = 0; i < NLFO; i++) lfos[i].on();
    for (int i = 0; i < NENV; i++) envs[i].on();
    for (int i = 0; i < NOSC; i++) {
        if (!data->oscs[i].free)
            oscs[i].reset();
    }
}

void rogueVoice::off(unsigned char velocity) {
    // trigger off
    for (int i = 0; i < NLFO; i++) lfos[i].off();
    for (int i = 0; i < NENV; i++) envs[i].off();

    //Mark the voice to be turned off later. It may not be set to
    //INVALID_KEY yet, because the release sound still needs to be
    //rendered.  m_key is finally set to INVALID_KEY by 'render' when
    //env < SILENCE
    m_key = SUSTAIN;
}

void rogueVoice::runLFO(int i, uint32_t from, uint32_t to) {
    LFOData& lfoData = data->lfos[i];
    rogueLFO& lfo = lfos[i];
    float v = 0.0f;
    if (lfoData.on) {
        float a = lfoData.attack * sample_rate;
        float d = lfoData.decay * sample_rate;
        lfo.lfo.setType(lfoData.type);
        lfo.lfo.setEnv(a, d);
        lfo.lfo.setFreq(lfoData.freq);
        lfo.lfo.setSymmetry(lfoData.symmetry);
        lfo.lfo.setHumanize(lfoData.humanize);
        lfo.lfo.setResetType(lfoData.reset_type);
        v = lfo.lfo.tick(to - from);
    }
    lfo.last = lfo.current;
    lfo.current = 0.0f;
}

void rogueVoice::runEnv(int i, uint32_t from, uint32_t to) {
    EnvData& envData = data->envs[i];
    rogueEnv& env = envs[i];
    float v = 0.0f;
    if (envData.on) {
        float a = envData.attack * sample_rate;
        float d = envData.decay * sample_rate;
        float s = envData.sustain;
        float r = envData.release * sample_rate;
        // TODO pre-delay
        // TODO hold
        // TODO retrigger
        // TODO modulation
        env.adsr.setADSR(a, d, s, r);
        v = env.adsr.tick(to - from);
    }
    env.last = env.current;
    env.current = v;
}

void rogueVoice::runOsc(int i, uint32_t from, uint32_t to) {
    OscData& oscData = data->oscs[i];
    rogueOsc& osc = oscs[i];
    if (oscData.on) {
        float f = 440.0;
        if (oscData.tracking) {
            f = midi2hz(float(m_key) + oscData.coarse + oscData.fine);
        }
        f *= oscData.ratio;
        osc.osc.setType(oscData.type);
        osc.osc.setFreq(f);
        osc.osc.setParams(oscData.param1, oscData.param2);
        osc.osc.process(osc.buffer + from, to - from);
        float v = oscData.volume; // TODO vel_to_vol
        if (oscData.inv) {
            v = -1.0f * v;
        }
        if (oscData.level_a > 0.0f) {
            float vv = v * oscData.level_a;
            for (int i = from; i < to; i++) {
                bus_a[i] += vv * osc.buffer[i];
            }
        }
        if (oscData.level_b > 0.0f) {
            float vv = v * oscData.level_b;
            for (int i = from; i < to; i++) {
                bus_b[i] += vv * osc.buffer[i];
            }
        }
    }
}

void rogueVoice::runFilter(int i, uint32_t from, uint32_t to) {
    FilterData& filterData = data->filters[i];
    rogueFilter& filter = filters[i];
    if (filterData.on) {
        int type = filterData.type;
        float f = filterData.freq; // TODO key_to_f, vel_to_f
        float* source;
        switch (filterData.source) {
        case 0:
            source = bus_a; break;
        case 1:
            source = bus_b; break;
        case 2:
            source = filters[0].buffer;
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
    }
}

void rogueVoice::render(uint32_t from, uint32_t to) {
    if (m_key == daps::INVALID_KEY) {
        return;
    }

    // reset buses
    for (int i = from; i < to; i++) {
        bus_a[i] = bus_b[i] = 0.0f;
    }

    // run elements
    for (int i = 0; i < NLFO; i++) runLFO(i, from, to);
    for (int i = 0; i < NENV; i++) runEnv(i, from, to);
    for (int i = 0; i < NOSC; i++) runOsc(i, from, to);
    for (int i = 0; i < NDCF; i++) runFilter(i, from, to);

    // copy buses and filters to out
    for (int i = from; i < to; i++) {
        // TODO
    }

    env = envs[0].current;
    if (env < SILENCE) {
        m_key = daps::INVALID_KEY;
    }
}

void rogueVoice::reset() {
    env = 0.0f;
    volume = 1.0f;
    sustain = 0;
    m_key = daps::INVALID_KEY;
}


}
