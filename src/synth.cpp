/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * uses code from https://github.com/rekado/lv2-mdaPiano
 */

#include "synth.h"

namespace rogue {

rogueSynth::rogueSynth(double rate)
  : lvtk::Synth<rogueVoice, rogueSynth>(p_n_ports, p_control) {

    sample_rate = rate;

    for(int i = 0; i < NVOICES; i++) {
        voices[i] = new rogueVoice(rate, &data);
        add_voices(voices[i]);
    }

    add_audio_outputs(p_left, p_right);
}

unsigned rogueSynth::find_free_voice(unsigned char key, unsigned char velocity) {
    //is this a retriggered note?
    for (int i = 0; i < NVOICES; i++) {
        if ((voices[i]->get_key() == key) && (voices[i]->is_sustained())) {
            return i;
        }
    }

    //take the next free voice if
    // ... notes are sustained but not this new one
    // ... notes are not sustained
    for (int i = 0; i < NVOICES; i++) {
        if (voices[i]->get_key() == lvtk::INVALID_KEY) {
            return i;
        }
    }

    //TODO: steal quietest note if all voices are used up
    return 0;
}


void rogueSynth::set_volume(float value) {
    for (int v=0; v < NVOICES; v++) {
        voices[v]->set_volume(value);
    }

}

//parameter change
void rogueSynth::update() {
    // TODO scale dB parameters
    data.bus_a_level = v(p_bus_a_level); // scale
    data.bus_a_pan   = v(p_bus_a_pan);
    data.bus_b_level = v(p_bus_b_level); // scale
    data.bus_b_pan   = v(p_bus_b_pan);
    data.volume      = v(p_volume); // scale
    data.glide_time  = v(p_glide_time);
    data.bend_range  = v(p_bend_range);

    const float rate = sample_rate;

    // XXX skip conf copying if element is off?

    // oscs
    for (int i = 0; i < NOSC; i++) {
        int off = i * OSC_OFF;
        data.oscs[i].on          = v(p_osc1_on + off);
        data.oscs[i].type        = v(p_osc1_type + off);
        data.oscs[i].inv         = v(p_osc1_inv + off);
        data.oscs[i].free        = v(p_osc1_free + off);
        data.oscs[i].tracking    = v(p_osc1_tracking + off);
        data.oscs[i].ratio       = v(p_osc1_ratio + off);
        data.oscs[i].coarse      = v(p_osc1_coarse + off);
        data.oscs[i].fine        = v(p_osc1_fine + off);
        data.oscs[i].param1      = v(p_osc1_param1 + off);
        data.oscs[i].param2      = v(p_osc1_param2 + off);
        data.oscs[i].level_a     = v(p_osc1_level_a + off); // scale
        data.oscs[i].level_b     = v(p_osc1_level_b + off); // scale
        data.oscs[i].level       = v(p_osc1_level + off); // scale

        data.oscs[i].vel_to_vol  = v(p_osc1_vel_to_vol + off);
    }

    // filters
    for (int i = 0; i < NDCF; i++) {
        int off = i * DCF_OFF;
        data.filters[i].on       = v(p_filter1_on + off);
        data.filters[i].type     = v(p_filter1_type + off);
        data.filters[i].freq     = v(p_filter1_freq + off);
        data.filters[i].q        = v(p_filter1_q + off);
        data.filters[i].distortion = v(p_filter1_distortion + off);
        data.filters[i].level    = v(p_filter1_level + off); // scale
        data.filters[i].pan      = v(p_filter1_pan + off);

        data.filters[i].key_to_f = v(p_filter1_key_to_f + off);
        data.filters[i].vel_to_f = v(p_filter1_vel_to_f + off);
    }

    // lfos
    for (int i = 0; i < NLFO; i++) {
        int off = i * LFO_OFF;
        data.lfos[i].on          = v(p_lfo1_on + off);
        data.lfos[i].type        = v(p_lfo1_type + off);
        data.lfos[i].inv         = v(p_lfo1_inv + off);
        data.lfos[i].reset_type  = v(p_lfo1_reset_type + off);
        data.lfos[i].freq        = v(p_lfo1_freq + off);
        data.lfos[i].symmetry    = v(p_lfo1_symmetry + off);
        data.lfos[i].attack      = v(p_lfo1_attack + off) * rate;
        data.lfos[i].decay       = v(p_lfo1_decay + off) * rate;
        data.lfos[i].humanize    = v(p_lfo1_humanize + off);
    }

    // envs
    for (int i = 0; i < NENV; i++) {
        int off = i * ENV_OFF;
        data.envs[i].on          = v(p_env1_on + off);
        data.envs[i].pre_delay   = v(p_env1_pre_delay + off) * rate;
        data.envs[i].attack      = v(p_env1_attack + off) * rate;
        data.envs[i].hold        = v(p_env1_hold + off) * rate;
        data.envs[i].decay       = v(p_env1_decay + off) * rate;
        data.envs[i].sustain     = v(p_env1_sustain + off);
        data.envs[i].release     = v(p_env1_release + off) * rate;
        data.envs[i].retrigger   = v(p_env1_retrigger + off);
    }

    // mods
    for (int i = 0; i < NMOD; i++) {
        int off = i * MOD_OFF;
        data.mods[i].src         = v(p_mod1_src + off);
        data.mods[i].target      = v(p_mod1_target + off);
        data.mods[i].amount      = v(p_mod1_amount + off);
    }
}

void rogueSynth::pre_process(uint32_t from, uint32_t to) {
    update();
}

// TODO post_process with effects

void rogueSynth::handle_midi(uint32_t size, unsigned char* data) {

    //discard invalid midi messages
    if (size != 3) {
        return;
    }

    //receive on all channels
    switch(data[0] & 0xf0) {
    case 0x80: //note off
        for (int i = 0; i < NVOICES; ++i) {
            if (voices[i]->get_key() == data[1]) {
                voices[i]->off(data[2]);
                break;
           }
        }
        break;

    case 0x90: //note on
        voices[ find_free_voice(data[1], data[2]) ]->on(data[1], data[2]);
        break;

    case 0xE0:
        // TODO: pitch bend
        break;

    //controller
    case 0xB0:
        switch (data[1]) {
        case 0x01:  //mod wheel
        case 0x43:  //soft pedal
            // TODO
            break;

        case 0x07:  //volume
            set_volume(0.00002f * (float)(data[2] * data[2]));
            break;

        case 0x40:  //sustain pedal
        case 0x42:  //sostenuto pedal
            // TODO
            break;

        //all sound off
        case 0x78:
        //all notes off
        case 0x7b:
            for (int v = 0; v < NVOICES; v++) {
                voices[v]->reset();
            }
            break;

        default: break;
        }
        break;

    default: break;
    }
}

static int _ = rogueSynth::register_class(p_uri);

}
