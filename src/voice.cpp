/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * uses code from https://github.com/rekado/lv2-mdaPiano
 */

#include "voice.h"

namespace rogue {

rogueVoice::rogueVoice(double rate, SynthData* data) : data(data) {
    sample_rate = rate;
    for (int i = 0; i < 4; i++) oscs[i] = rogueOsc();
    for (int i = 0; i < 2; i++) filters[i] = rogueFilter();
    for (int i = 0; i < 3; i++) lfos[i] = rogueLfo();
    for (int i = 0; i < 5; i++) envs[i] = rogueEnv();

    // TODO set sample_rate of elements
}

void rogueVoice::on(unsigned char key, unsigned char velocity) {
    // store key that turned this voice on (used in 'get_key')
    m_key = key;

    if (velocity > 0) {
        // TODO
    } else {
        // some keyboards send note off events as 'note on' with velocity 0
        release(0);
    }
}

void rogueVoice::release(unsigned char velocity) {
    //Mark the voice to be turned off later. It may not be set to
    //INVALID_KEY yet, because the release sound still needs to be
    //rendered.  m_key is finally set to INVALID_KEY by 'render' when
    //env < SILENCE
    m_key = SUSTAIN;
}

void rogueVoice::render(uint32_t from, uint32_t to) {
    if (m_key == daps::INVALID_KEY) {
        return;
    }

    for (uint32_t frame = from; frame < to; ++frame) {
        // TODO
    }

    if (env < SILENCE) {
        m_key = daps::INVALID_KEY;
    }
}

void rogueVoice::reset() {
    env = 0.0f;
    volume = 0.2f;
    sustain = 0;
    m_key = daps::INVALID_KEY;
}


}
