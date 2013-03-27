/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * This header is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License,
 * or (at your option) any later version.
 *
 * This header is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA.
 */

struct DelaylineData {

};

struct EnvelopeData {
    bool on;
    float pre_delay;
    float attack;
    float hold;
    float decay;
    float sustain;
    float release;
    bool env_retrigger;
    float vel_to_vol;

    // modulation
    float key_to_speed;
    float vel_to_speed;
};

struct FilterData {
    bool on;
    int type;
    int destination;
    float freq;
    float q;
    float distortion;
    float volume;
    float pan;

    // modulation
    float env_to_filter;
    float vel_to_filter;
    float key_to_filter;
    float mod_to_filter;
};

struct LFOData {
    bool on;
    int type;
    int reset_type;
    float frequency;
    float amount;
    float symmetry;
    float attack;
    float decay;
    float humanization;

    // modulation
    float key_to_freq;
};

struct OscillatorData {
    bool on;
    int type;
    int destination;
    bool inv; // output inversion
    bool free; // phase reset
    bool tracking; // pitch tracking
    float ratio;
    float coarse; //  tune
    float fine; // tune
    float volume;
};

