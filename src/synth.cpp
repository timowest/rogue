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
  : daps::Synth<rogueVoice, rogueSynth>(p_n_ports, p_control) {

    sample_rate = rate;
    sustain = 0;

    for(int i = 0; i < NVOICES; i++) {
        voices[i] = new rogueVoice(rate, &data);
        add_voices(voices[i]);
    }

    add_audio_outputs(p_left, p_right);
}

unsigned rogueSynth::find_free_voice(unsigned char key, unsigned char velocity) {
    //is this a retriggered note during sustain?
    if (sustain) {
        for (int i = 0; i < NVOICES; i++) {
            if ((voices[i]->get_key() == key) && (voices[i]->is_sustained())) {
                return i;
            }
        }
    }

    //take the next free voice if
    // ... notes are sustained but not this new one
    // ... notes are not sustained
    for (int i = 0; i < NVOICES; i++) {
        if (voices[i]->get_key() == daps::INVALID_KEY) {
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
    data.bus_a_level = *p(p_bus_a_level);
    data.bus_a_pan   = *p(p_bus_a_pan);
    data.bus_b_level = *p(p_bus_b_level);
    data.bus_b_pan   = *p(p_bus_b_pan);
    data.volume      = *p(p_volume);
    data.glide_time  = *p(p_glide_time);
    data.bend_range  = *p(p_bend_range);

    const float rate = sample_rate;

    // oscs
    for (int i = 0; i < NOSC; i++) {
        int off = i * 9;
        data.oscs[i].on          = *p(p_osc1_on + off);
        data.oscs[i].type        = *p(p_osc1_type + off);
        data.oscs[i].inv         = *p(p_osc1_inv + off);
        data.oscs[i].free        = *p(p_osc1_free + off);
        data.oscs[i].tracking    = *p(p_osc1_tracking + off);
        data.oscs[i].ratio       = *p(p_osc1_ratio + off);
        data.oscs[i].coarse      = *p(p_osc1_coarse + off);
        data.oscs[i].fine        = *p(p_osc1_fine + off);
        data.oscs[i].volume      = *p(p_osc1_volume + off);

        data.oscs[i].vel_to_vol  = *p(p_osc1_vel_to_vol + off);
    }

    // filters
    for (int i = 0; i < NDCF; i++) {
        int off = i * 10;
        data.filters[i].on       = *p(p_filter1_on + off);
        data.filters[i].type     = *p(p_filter1_type + off);
        data.filters[i].freq     = *p(p_filter1_freq + off);
        data.filters[i].q        = *p(p_filter1_q + off);
        data.filters[i].distortion = *p(p_filter1_distortion + off);
        data.filters[i].level    = *p(p_filter1_level + off);
        data.filters[i].pan      = *p(p_filter1_pan + off);

        data.filters[i].key_to_f = *p(p_filter1_key_to_f + off);
        data.filters[i].vel_to_f = *p(p_filter1_vel_to_f + off);
    }

    // lfos
    for (int i = 0; i < NLFO; i++) {
        int off = i * 10;
        data.lfos[i].on          = *p(p_lfo1_on + off);
        data.lfos[i].type        = *p(p_lfo1_type + off);
        data.lfos[i].reset_type  = *p(p_lfo1_reset_type + off);
        data.lfos[i].freq        = *p(p_lfo1_freq + off);
        data.lfos[i].symmetry    = *p(p_lfo1_symmetry + off);
        data.lfos[i].attack      = *p(p_lfo1_attack + off) * rate;
        data.lfos[i].decay       = *p(p_lfo1_decay + off) * rate;
        data.lfos[i].humanize    = *p(p_lfo1_humanize + off);

        data.lfos[i].key_to_f    = *p(p_lfo1_key_to_f + off);
    }

    // envs
    for (int i = 0; i < NENV; i++) {
        int off = i * 11;
        data.envs[i].on          = *p(p_env1_on + off);
        data.envs[i].pre_delay   = *p(p_env1_pre_delay + off) * rate;
        data.envs[i].attack      = *p(p_env1_attack + off) * rate;
        data.envs[i].hold        = *p(p_env1_hold + off) * rate;
        data.envs[i].decay       = *p(p_env1_decay + off) * rate;
        data.envs[i].sustain     = *p(p_env1_sustain + off);
        data.envs[i].release     = *p(p_env1_release + off) * rate;
        data.envs[i].retrigger   = *p(p_env1_retrigger + off);

        data.envs[i].vel_to_vol  = *p(p_env1_vel_to_vol + off);
        data.envs[i].key_to_speed = *p(p_env1_key_to_speed + off);
        data.envs[i].vel_to_speed = *p(p_env1_vel_to_speed + off);
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
        for (unsigned i = 0; i < NVOICES; ++i) {
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
            sustain = data[2] & 0x40;

            for (int i = 0; i < NVOICES; ++i) {
                voices[i]->set_sustain(sustain);
                //if pedal was released: dampen sustained notes
                if((sustain == 0) && (voices[i]->is_sustained())) {
                    voices[i]->off(0);
                }
            }
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
