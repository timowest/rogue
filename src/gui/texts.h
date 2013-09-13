#ifndef QTTEXTS_HPP
#define QTTEXTS_HPP

// TODO replace with vectors
#define CHARS static const char*

CHARS modes[] = {"Poly", "Mono", "Legato"};

// checkbox content
CHARS osc_types[] = {
        // V
        "VA Saw", "VA Tri Saw", "VA Pulse",
        "PD Saw", "PD Square", "PD Pulse", "PD Double Sine", "PD Saw Pulse", "PD Res 1", "PD Res 2", "PD Res 3", "PD Half Sine",
        "Saw", "Double Saw", "Tri", "Pulse", "Pulse Saw", "Slope", "Alpha 1", "Alpha 2", "Exp",
        "FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8",
        // AS
        "AS Saw", "AS Square", "AS Triangle",
        // Noise
        "Noise", "Pink Noise", "LP Noise", "BP Noise"};

CHARS filter_types[] = {"LP 24dB", "LP 18dB", "LP 12dB", "LP 6dB", "HP 24dB",
        "BP 12dB", "BP 18dB", "Notch",
        "SVF LP", "SVF HP", "SVF BP", "SVF Notch",
        "Comb"};

CHARS filter_sources[] = {"Bus A", "Bus B", "Filter1"};

CHARS lfo_types[] = {"Sin", "Tri", "Saw", "Pulse", "S/H", "Noise"};

CHARS lfo_reset_types[] = {"Poly", "Free", "Mono"};

// labels

CHARS nums[] = {"1", "2", "3", "4", "5", "6"};

CHARS osc_labels[] = {"OSC 1", "OSC 2", "OSC 3", "OSC 4"};

CHARS filter_labels[] = {"Filter 1", "Filter 2"};

CHARS lfo_labels[] = {"LFO 1", "LFO 2", "LFO 3"};

CHARS env_labels[] = {"Env 1", "Env 2", "Env 3", "Env 4", "Env 5"};

CHARS mod_src_labels[] = {
        "-",
        "Mod", "Press", "Key", "Velo",
        "LFO 1", "LFO 1+", "LFO 2", "LFO 2+", "LFO 3", "LFO 3+", "LFO 4", "LFO 4+",
        "Env 1", "Env 2", "Env 3", "Env 4"};

CHARS mod_target_labels[]  = {
        "-",
        // osc
        "OSC 1 Pitch", "OSC 1 Mod", "OSC 1 PWM", "OSC 1 Amp",
        "OSC 2 Pitch", "OSC 2 Mod", "OSC 2 PWM", "OSC 2 Amp",
        "OSC 3 Pitch", "OSC 3 Mod", "OSC 3 PWM", "OSC 3 Amp",
        "OSC 4 Pitch", "OSC 4 Mod", "OSC 4 PWM", "OSC 4 Amp",
        // dcf
        "Flt 1 Freq", "Flt 1 Q", "Flt 1 Pan", "Flt 1 Amp",
        "Flt 2 Freq", "Flt 2 Q", "Flt 2 Pan", "Flt 2 Amp",
        // lfo
        "LFO 1 Sp", "LFO 1 Amp",
        "LFO 2 Sp", "LFO 2 Amp",
        "LFO 3 Sp", "LFO 3 Amp",
        "LFO 4 Sp", "LFO 4 Amp",
        // env
        "Env 1 Sp", "Env 1 Amp",
        "Env 2 Sp", "Env 2 Amp",
        "Env 3 Sp", "Env 3 Amp",
        "Env 4 Sp", "Env 4 Amp",
        // bus
        "Bus A Pan", "Bus B Pan"};

CHARS out_mod[] = {"-", "Add", "RM", "AM"};

#endif
