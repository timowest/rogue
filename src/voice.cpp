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

    // init elements
    for (int i = 0; i < NOSC; i++) oscs[i] = Osc();
    for (int i = 0; i < NDCF; i++) filters[i] = Filter();
    for (int i = 0; i < NLFO; i++) lfos[i] = LFO();
    for (int i = 0; i < NENV; i++) envs[i] = Env();

    // set sample rate
    for (int i = 0; i < NOSC; i++) oscs[i].setSamplerate(rate);
    for (int i = 0; i < NDCF; i++) filters[i].setSamplerate(rate);
    for (int i = 0; i < NLFO; i++) lfos[0].setSamplerate(rate);

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
        if (!data->oscs[i].free) oscs[i].resetPhase();
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

static float amp_mod(float v, float amt, float mval) {
    if (amt > 0) {
        return v * (1.0f - amt + amt * mval);
    } else {
        return v * (1.0f + amt * mval);
    }
}

static float pitch_mod(float v, float amt, float mval) {
    return v + amt * mval;
}

template<class Function>
float rogueVoice::modulate(float init, int target, Function fn) {
    float v = init;
    for (int i = 0; i < NMOD; i++) {
        if (data->mods[i].target == target) {
            ModulationData& modData = data->mods[i];
            v = fn(v, modData.amount, mod[modData.src]);
        }
    }
    return v;
}

void rogueVoice::configLFO(int i) {
    LFOData& lfoData = data->lfos[i];
    LFO& lfo = lfos[i];

    float f = lfoData.freq;
    // key to f
    f *= modulate(1.0f, M_LFO1_S + 2 * i, amp_mod);

    // NOTE: lfos can't modulate each other's freq
    lfo.lfo.setType(lfoData.type);
    lfo.lfo.setFreq(f);
    lfo.lfo.setWidth(lfoData.width);
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
        v *= modulate(1.0f, M_LFO1_AMP + 2 * i, amp_mod);
    }
    // update mod values
    mod[M_LFO1_BI + 2*i] = v;
    mod[M_LFO1_UN + 2*i] = 0.5f * v + 0.5f;

    lfo.last = lfo.current;
    lfo.current = v;
}

void rogueVoice::configEnv(int i) {
    EnvData& envData = data->envs[i];
    Env& env = envs[i];

    // NOTE: envelopes can't modulate each other's speed
    float f = 1.0f / modulate(1.0f, M_ENV1_S + 2 * i, amp_mod); // TODO use different modulation algorithm
    float a = envData.attack / f;
    float h = envData.hold / f;
    float d = envData.decay / f;
    float s = envData.sustain;
    float r = envData.release / f;

    // TODO retrigger
    env.env.setCurve(envData.curve);
    env.env.setPredelay(envData.pre_delay / f);
    env.env.setAHDSR(a, h, d, s, r);
}

void rogueVoice::runEnv(int i, uint32_t from, uint32_t to) {
    EnvData& envData = data->envs[i];
    Env& env = envs[i];
    float v = 0.0f;
    if (envData.on) {
        v = env.env.tick(to - from);
        // amp modulation
        v *= modulate(1.0f, M_ENV1_AMP + 2 * i, amp_mod);
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
        float pmod = modulate(0.0f, M_OSC1_P + 4 * i, pitch_mod);
        if (oscData.tracking) {
            f = midi2hz(float(m_key) + oscData.coarse + oscData.fine + pmod);
        } else if (pmod > 0.0f) {
            f = midi2hz(69.0f + pmod);
        }
        f *= oscData.ratio;

        // pulse width modulation
        float width = oscData.width * modulate(1.0f, M_OSC1_PWM + 4 * i, amp_mod);

        // TODO pm
        // TODO sync

        // process
        osc.process(oscData.type, f, oscData.start, osc.width_prev, width, osc.buffer + from, to - from);

        // amp modulation
        float v = oscData.level;
        if (oscData.inv) {
            v *= -1.0f;
        }
        if (oscData.vel_to_vol > 0.0f) {
            v *= 1.0 - oscData.vel_to_vol + oscData.vel_to_vol * midi2f(m_velocity);
        }

        v *= modulate(1.0f, M_OSC1_AMP + 4 * i, amp_mod);
        float step = (v - osc.prev_level) / float(to - from);
        float l = osc.prev_level;
        for (int i = from; i < to; i++) {
            osc.buffer[i] *= l;
            l += step;
        }
        osc.prev_level = v;

        // audio output modulation
        if (oscData.out_mod > 0) {
            float* in = oscs[oscData.input].buffer;
            switch (oscData.out_mod) {
            case 1: // Add
                for (int i = from; i < to; i++) {
                    osc.buffer[i] += in[i];
                }
                break;
            case 2: // RM
                for (int i = from; i < to; i++) {
                    osc.buffer[i] *= in[i];
                }
                break;
            case 3: // AM
                for (int i = from; i < to; i++) {
                    osc.buffer[i] *= 0.5f * (in[i] + 1.0f);
                }
                break;
            }
        }

        // copy to buffers
        for (int i = from; i < to; i++) {
            bus_a[i] += oscData.level_a * osc.buffer[i];
            bus_b[i] += oscData.level_b * osc.buffer[i];
        }

        osc.width_prev = width;
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
        f *= modulate(1.0f, M_DCF1_F + 4 * i, amp_mod); // TODO use different modulation algorithm here

        // res modulation
        float q = filterData.q;
        q *= modulate(1.0f, M_DCF1_Q + 4 * i, amp_mod);

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
        float v = modulate(1.0f, M_DCF1_AMP + 4 * i, amp_mod);
        float step = (v - filter.prev_level) / float(to - from);
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

    float* left = p(p_left);
    float* right = p(p_right);

    // pan config (not interpolated)
    // bus a, bus b, filter 1, filter 2
    float left_p[] = {data->bus_a_level * (1.0f - data->bus_a_pan),
                      data->bus_b_level * (1.0f - data->bus_b_pan),
                      data->filters[0].level * (1.0f - data->filters[0].pan),
                      data->filters[1].level * (1.0f - data->filters[1].pan)};
    float right_p[] = {data->bus_a_level * data->bus_a_pan,
                       data->bus_b_level * data->bus_b_pan,
                       data->filters[0].level * data->filters[0].pan,
                       data->filters[1].level * data->filters[1].pan};

    // TODO bus a pan modulation
    // TODO bus b pan modulation
    // TODO filter1 pan modulation
    // TODO filter2 pan modulation

    // amp modulation
    float e_from = envs[0].last;
    float e_step = (envs[0].current - e_from) / float(to - from);

    // copy buffers
    for (int i = from; i < to; i++) {
        for (int j = 0; j < 4; j++) {
            float sample = data->volume * e_from * buffers[j][i];
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
    std::cout << "reset " << int(m_key) << std::endl;
    volume = 1.0f;
    m_key = lvtk::INVALID_KEY;
    in_sustain = false;
    std::memset(mod, 0, sizeof(float) * NMOD);

    for (int i = 0; i < NLFO; i++) lfos[i].reset();
    for (int i = 0; i < NENV; i++) envs[i].reset();
    for (int i = 0; i < NOSC; i++) oscs[i].reset();
    for (int i = 0; i < NDCF; i++) filters[i].reset();
}


}
