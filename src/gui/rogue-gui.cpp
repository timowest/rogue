/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <gtkmm.h>
#include <lv2gui.hpp>

#include "gui/combo.h"
#include "gui/knob.h"
#include "gui/toggle.h"
#include "gui/panel.h"
#include "analogue.peg"
#include "analogue-meta.h"

#include <stdio.h>
#include <string.h>

using namespace sigc;
using namespace Gtk;

class rogueGUI : public LV2::GUI<rogueGUI, LV2::URIMap<true>, LV2::WriteMIDI<false> > {
  public:
    rogueGUI(const std::string& URI);
    Widget* createOSC1();
    Widget* createOSC2();
    Widget* createNoise();
    Widget* createLFO1();
    Widget* createLFO2();
    Widget* createFilter1();
    Widget* createFilter2();
    Widget* createAmp1();
    Widget* createAmp2();
    Widget* createFilter1Env();
    Widget* createFilter2Env();
    Widget* createAmp1Env();
    Widget* createAmp2Env();
    Widget* createFlanger();
    Widget* createDelay();
    Widget* createReverb();
    void control(Table* table, const char* label, int port_index, int left, int top);
    Widget* smallFrame(const char* label, Table* content);
    Widget* frame(const char* label, int toggle, Table* content);
    Alignment* align(Widget* widget);
    bool isEffect(int i);
    bool isToggle(int i);
    bool isPower(int i);
    bool isBypass(int i);
    bool isOSCType(int i);
    bool isFilterType(int i);
    bool isEnvControl(int i);
    bool isModControl(int i);
    void port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer);
    void change_status_bar(uint32_t port, float value);

  protected:
    VBox mainBox;
    Statusbar statusbar;
    char statusBarText[100];
    //Knob *scales[p_n_ports - 3];
    Changeable *scales[p_n_ports - 3];
};

// implementation

rogueGUI::rogueGUI(const std::string& URI) {
    std::cout << "starting GUI" <<std::endl;

    int control_ports = p_n_ports - 3;

    // TODO use table for the widget types
    //initialize sliders
    for (int i = 0; i < control_ports; i++) {
        if (isOSCType(i)) {
            scales[i] = manage(new OSCTypeComboBox());
        } else if (isFilterType(i)) {
            scales[i] = manage(new FilterTypeComboBox());
        } else if (isPower(i)) {
            scales[i] = manage(new Toggle());
        } else if (isBypass(i)) {
            scales[i] = manage(new Toggle(true));
        } else {
            Knob* knob = new Knob(p_port_meta[i].min, p_port_meta[i].max, p_port_meta[i].step);
            if (isEnvControl(i)) {
                // small
                knob->set_size(30);
                knob->set_radius(10);
            } else if (isModControl(i) || isEffect(i)) {
               // medium
               knob->set_radius(12.0);
            }
            scales[i] = manage(knob);
        }
    }

    //connect widgets to control ports (change control values when sliders are moved)
    for (int i = 0; i < control_ports; i++) {
        slot<void> slot1 = compose(bind<0>(mem_fun(*this, &rogueGUI::write_control), i + 3),
            mem_fun(*scales[i], &Changeable::get_value));
        slot<void> slot2 = compose(bind<0>(mem_fun(*this, &rogueGUI::change_status_bar), i + 3),
            mem_fun(*scales[i], &Changeable::get_value));
        scales[i]->connect(slot1);
        if (!isOSCType(i) && !isFilterType(i) && !isToggle(i)) {
            scales[i]->connect(slot2);
        }
    }

    Table* block1 = manage(new Table(2,4));
    block1->attach(*createOSC1(),    0, 1, 1, 2);
    block1->attach(*createFilter1(), 1, 2, 1, 2);
    block1->attach(*createAmp1(),    2, 3, 1, 2);
    block1->attach(*createLFO1(),    3, 4, 1, 2);
    block1->attach(*createLFO2(),    0, 1, 2, 3);
    block1->attach(*createOSC2(),    1, 2, 2, 3);
    block1->attach(*createFilter2(), 2, 3, 2, 3);
    block1->attach(*createAmp2(),    3, 4, 2, 3);
    mainBox.pack_start(*align(block1));

    HBox* block3 = manage(new HBox());
    block3->pack_start(*createFilter1Env());
    block3->pack_start(*createFilter2Env());
    block3->pack_start(*createAmp1Env());
    block3->pack_start(*createAmp2Env());
    mainBox.pack_start(*align(block3));

    HBox* block4 = manage(new HBox());
    block4->pack_start(*createNoise());
    block4->pack_start(*createFlanger());
    block4->pack_start(*createDelay());
    block4->pack_start(*createReverb());
    mainBox.pack_start(*align(block4));

    HBox* header = manage(new HBox());
    header->pack_start(*manage(new Image("analogue.png")));
    header->pack_end(*scales[p_amp_output - 3]->get_widget());
    header->set_border_width(5);
    mainBox.pack_start(*align(header));

    mainBox.pack_end(statusbar);

    add(*align(&mainBox));

    std::cout << "GUI ready" <<std::endl;
}

Widget* rogueGUI::createOSC1() {
    Table* table = manage(new Table(4,5));
    // row 1
    control(table, "Type", p_osc1_type, 0, 1);
    control(table, "Tune", p_osc1_tune, 1, 1);
    control(table, "PW", p_osc1_width, 2, 1);
    control(table, "Kbd", p_osc1_kbd, 3, 1);
    //addControl(table, "Sync", p_osc1_type, 0, 1);
    // row 2
    control(table, "Level", p_osc1_level, 0, 3);
    control(table, "Finetune", p_osc1_finetune, 1, 3);
    control(table, "LFO1", p_osc1_lfo_to_w, 2, 3);
    control(table, "LFO1", p_osc1_lfo_to_p, 3, 3);
    control(table, "F1 F2", p_osc1_f1_to_f2, 4, 3);
    return frame("OSC1", p_osc1_power, table);
}

Widget* rogueGUI::createOSC2() {
    Table* table = manage(new Table(6,4));
    // row 1
    control(table, "Type", p_osc2_type, 0, 1);
    control(table, "Tune", p_osc2_tune, 1, 1);
    control(table, "PW", p_osc2_width, 2, 1);
    control(table, "Kbd", p_osc2_kbd, 3, 1);
    //addControl(table, "Sync", p_osc1_type, 0, 1);
    // row 2
    control(table, "Level", p_osc2_level, 0, 3);
    control(table, "Finetune", p_osc2_finetune, 1, 3);
    control(table, "LFO2", p_osc2_lfo_to_w, 2, 3);
    control(table, "LFO2", p_osc2_lfo_to_p, 3, 3);
    control(table, "F1 F2", p_osc2_f1_to_f2, 4, 3);
    return frame("OSC2", p_osc2_power, table);
}

Widget* rogueGUI::createNoise() {
    Table* table = manage(new Table(2, 3));
    control(table, "Color", p_noise_color, 0, 1);
    control(table, "F1 F2", p_noise_f1_to_f2, 1, 1);
    control(table, "Level", p_noise_level, 2, 1);
    return frame("Noise", p_noise_power, table);
}

Widget* rogueGUI::createLFO1() {
    Table* table = manage(new Table(2, 5));
    control(table, "Type", p_lfo1_type, 0, 1);
    control(table, "Freq", p_lfo1_freq, 1, 1);
    control(table, "Delay", p_lfo1_delay, 2, 1);
    control(table, "Fade In", p_lfo1_fade_in, 3, 1);
    control(table, "Width", p_lfo1_width, 4, 1);
    return frame("LFO1", p_lfo1_power, table);
}

Widget* rogueGUI::createLFO2() {
    Table* table = manage(new Table(2, 5));
    control(table, "Type", p_lfo2_type, 0, 1);
    control(table, "Freq", p_lfo2_freq, 1, 1);
    control(table, "Delay", p_lfo2_delay, 2, 1);
    control(table, "Fade In", p_lfo2_fade_in, 3, 1);
    control(table, "Width", p_lfo2_width, 4, 1);
    return frame("LFO2", p_lfo2_power, table);
}

Widget* rogueGUI::createFilter1() {
    Table* table = manage(new Table(4,5));
    // row 1
    control(table, "Type", p_filter1_type, 0, 1);
    control(table, "Kbd", p_filter1_kbd, 1, 1);
    control(table, "Cutoff", p_filter1_cutoff, 2, 1);
    control(table, "Q", p_filter1_q, 3, 1);
    control(table, "To F2", p_filter1_to_f2, 4, 1);
    // row 2
    // TODO : order
    control(table, "LFO1", p_filter1_lfo_to_f, 1, 3);
    control(table, "Env1", p_filter1_env_to_f, 2, 3);
    control(table, "LFO1", p_filter1_lfo_to_q, 3, 3);
    control(table, "Env1", p_filter1_env_to_q, 4, 3);
    return frame("Filter 1", p_filter1_bypass, table);
}

Widget* rogueGUI::createFilter2() {
    Table* table = manage(new Table(4,5));
    // row 1
    control(table, "Type", p_filter2_type, 0, 1);
    control(table, "Kbd", p_filter2_kbd, 1, 1);
    control(table, "Cutoff", p_filter2_cutoff, 2, 1);
    control(table, "Q", p_filter2_q, 3, 1);
    // row 2
    // TODO : order
    control(table, "LFO2", p_filter2_lfo_to_f, 1, 3);
    control(table, "Env2", p_filter2_env_to_f, 2, 3);
    control(table, "LFO2", p_filter2_lfo_to_q, 3, 3);
    control(table, "Env2", p_filter2_env_to_q, 4, 3);
    return frame("Filter 2", p_filter2_bypass, table);
}

Widget* rogueGUI::createAmp1() {
    Table* table = manage(new Table(4,4));
    // row 1
    control(table, "Kbd", p_amp1_kbd_to_level, 0, 1);
    control(table, "Level", p_amp1_level, 1, 1);
    control(table, "Kbd", p_amp1_kbd_to_pan, 2, 1);
    control(table, "Pan", p_amp1_pan, 3, 1);
    // row 2
    control(table, "LFO", p_amp1_lfo_to_level, 0, 3);
    // empty
    control(table, "LFO", p_amp1_lfo_to_pan, 2, 3);
    control(table, "Env", p_amp1_env_to_pan, 3, 3);
    return frame("Amp 1", p_amp1_power, table);
}

Widget* rogueGUI::createAmp2() {
    Table* table = manage(new Table(4,4));
    // row 1
    control(table, "Kbd", p_amp2_kbd_to_level, 0, 1);
    control(table, "Level", p_amp2_level, 1, 1);
    control(table, "Kbd", p_amp2_kbd_to_pan, 2, 1);
    control(table, "Pan", p_amp2_pan, 3, 1);
    // row 2
    control(table, "LFO", p_amp2_lfo_to_level, 0, 3);
    // empty
    control(table, "LFO", p_amp2_lfo_to_pan, 2, 3);
    control(table, "Env", p_amp2_env_to_pan, 3, 3);
    return frame("Amp 2", p_amp2_power, table);
}

Widget* rogueGUI::createFilter1Env() {
    Table* table = manage(new Table(2,4));
    control(table, "A", p_filter1_attack, 0, 1);
    control(table, "D", p_filter1_decay, 1, 1);
    control(table, "S", p_filter1_sustain, 2, 1);
    control(table, "R", p_filter1_release, 3, 1);
    return smallFrame("Filter1 Env", table);
}

Widget* rogueGUI::createFilter2Env() {
    Table* table = manage(new Table(2,4));
    control(table, "A", p_filter2_attack, 0, 1);
    control(table, "D", p_filter2_decay, 1, 1);
    control(table, "S", p_filter2_sustain, 2, 1);
    control(table, "R", p_filter2_release, 3, 1);
    return smallFrame("Filter2 Env", table);
}

Widget* rogueGUI::createAmp1Env() {
    Table* table = manage(new Table(2,4));
    control(table, "A", p_amp1_attack, 0, 1);
    control(table, "D", p_amp1_decay, 1, 1);
    control(table, "S", p_amp1_sustain, 2, 1);
    control(table, "R", p_amp1_release, 3, 1);
    return smallFrame("Amp1 Env", table);
}

Widget* rogueGUI::createAmp2Env() {
    Table* table = manage(new Table(2,4));
    control(table, "A", p_amp2_attack, 0, 1);
    control(table, "D", p_amp2_decay, 1, 1);
    control(table, "S", p_amp2_sustain, 2, 1);
    control(table, "R", p_amp2_release, 3, 1);
    return smallFrame("Amp2 Env", table);
}

Widget* rogueGUI::createFlanger() {
    Table* table = manage(new Table(2, 4));
    // TODO : invert as toggle on top
    //control(table, "Invert", p_effects_flanger_invert, 0, 1);
    control(table, "Speed", p_effects_flanger_speed, 0, 1);
    control(table, "Fb", p_effects_flanger_feedback, 1, 1);
    control(table, "Delay", p_effects_flanger_flange_delay, 2, 1);
    control(table, "Mix", p_effects_flanger_mix, 3, 1);
    return frame("Flanger", p_effects_flanger_bypass, table);
}

Widget* rogueGUI::createDelay() {
    Table* table = manage(new Table(2, 4));
    control(table, "Length", p_effects_delay_length, 0, 1);
    control(table, "Cutoff", p_effects_delay_cutoff, 1, 1);
    control(table, "Depth", p_effects_delay_depth, 2, 1);
    control(table, "Mix", p_effects_delay_mix, 3, 1);
    return frame("Delay", p_effects_delay_bypass, table);
}

Widget* rogueGUI::createReverb() {
    // TODO : more parameters
    Table* table = manage(new Table(2, 3));
    control(table, "Damp", p_effects_reverb_damp, 0, 1);
    control(table, "Size", p_effects_reverb_roomSize, 1, 1);
    control(table, "Mix", p_effects_reverb_mix, 2, 1);
    return frame("Reverb", p_effects_reverb_bypass, table);
}

void rogueGUI::control(Table* table, const char* label, int port_index, int left, int top) {
    table->attach(*scales[port_index - 3]->get_widget(), left, left + 1, top, top + 1);
    table->attach(*manage(new Label(label)), left, left + 1, top + 1, top + 2);
}

Widget* rogueGUI::smallFrame(const char* label, Table* content) {
    content->set_border_width(2);
    content->set_col_spacings(2);
    content->set_spacings(2);

    Frame* frame = manage(new Frame());
    frame->set_label_align(0.0f, 0.0f);
    frame->set_border_width(5);
    frame->set_label(label);
    frame->add(*content);

    Alignment* alignment = manage(new Alignment(0.0, 0.0, 1.0, 0.0));
    alignment->add(*frame);
    return alignment;
}

Widget* rogueGUI::frame(const char* label, int toggle, Table* content) {
    content->set_border_width(2);
    content->set_col_spacings(5);
    content->set_spacings(2);

    Panel* panel = manage(new Panel(label, scales[toggle - 3]->get_widget(), content));

    Alignment* alignment = manage(new Alignment(0.0, 0.0, 1.0, 0.0));
    alignment->add(*panel);
    return alignment;
}

Alignment* rogueGUI::align(Widget* widget) {
    Alignment* alignment = manage(new Alignment(0.0, 0.0, 0.0, 0.0));
    alignment->add(*widget);
    return alignment;
}

bool rogueGUI::isEffect(int i) {
    const char* symbol = p_port_meta[i].symbol;
    return strstr(symbol, "effects_");
}

bool rogueGUI::isToggle(int i) {
    const char* symbol = p_port_meta[i].symbol;
    return strstr(symbol, "_power") || strstr(symbol, "_bypass");
}

bool rogueGUI::isPower(int i) {
    const char* symbol = p_port_meta[i].symbol;
    return strstr(symbol, "_power");
}

bool rogueGUI::isBypass(int i) {
    const char* symbol = p_port_meta[i].symbol;
    return strstr(symbol, "_bypass");
}

bool rogueGUI::isOSCType(int i) {
    const char* symbol = p_port_meta[i].symbol;
    return (strstr(symbol, "osc") || strstr(symbol, "lfo")) && strstr(symbol, "_type");
}

bool rogueGUI::isFilterType(int i) {
    const char* symbol = p_port_meta[i].symbol;
    return strstr(symbol, "filter1_type") || strstr(symbol, "filter2_type");
}

bool rogueGUI::isEnvControl(int i) {
    const char* symbol = p_port_meta[i].symbol;
    return strstr(symbol, "_attack") || strstr(symbol, "_decay") || strstr(symbol, "_sustain") || strstr(symbol, "_release");
}

bool rogueGUI::isModControl(int i) {
    const char* symbol = p_port_meta[i].symbol;
    return strstr(symbol, "_kbd_") || strstr(symbol, "_lfo_") || strstr(symbol, "_env_");
}

void rogueGUI::port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer) {
    if (port > 2) {
        scales[port-3]->set_value(*static_cast<const float*>(buffer));
    }
}

void rogueGUI::change_status_bar(uint32_t port, float value) {
   if (p_port_meta[port-3].step >= 1.0f) {
       sprintf(statusBarText, "%s = %3.0f", p_port_meta[port-3].symbol, value);
   } else {
       sprintf(statusBarText, "%s = %3.3f", p_port_meta[port-3].symbol, value);
   }
   statusbar.remove_all_messages();
   statusbar.push(statusBarText);
}

static int _ = rogueGUI::register_class("http://www.github.com/timowest/rogue/gui");

