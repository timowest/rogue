/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdio.h>
#include <string.h>
#include <gtkmm.h>
#include <lvtk/gtkui.hpp>

#include "common.h"
#include "rogue.gen"
#include "gui/config.gen"
#include "gui/combo.h"
#include "gui/knob.h"
#include "gui/toggle.h"
#include "gui/panel.h"

using namespace sigc;
using namespace Gtk;

class rogueGUI : public lvtk::UI<rogueGUI, lvtk::GtkUI<true>, lvtk::URID<true> > {
  public:
    rogueGUI(const std::string& URI);
    Widget* createOSC(int i);
    Widget* createFilter(int i);
    Widget* createLFO(int i);
    Widget* createEnv(int i);
    Widget* smallFrame(const char* label, Table* content);
    Widget* frame(const char* label, int toggle, Table* content);
    Alignment* align(Widget* widget);
    void control(Table* table, const char* label, int port_index, int left, int top);
    void port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer);
    void change_status_bar(uint32_t port, float value);

  protected:
    VBox mainBox;
    Statusbar statusbar;
    char statusBarText[100];
    Changeable* scales[p_n_ports];
};

// implementation

rogueGUI::rogueGUI(const std::string& URI) {
    std::cout << "starting GUI" <<std::endl;

    //initialize sliders
    for (int i = 3; i < p_n_ports; i++) {
        switch (p_port_meta[i].type) {
        case KNOB:
            // TODO step
            Knob* knob = new Knob(p_port_meta[i].min, p_port_meta[i].max);
            // TODO size
            scales[i] = manage(knob);
            break;
        case TOGGLE:
            scales[i] = manage(new Toggle());
            break;
        case SELECT:
            // TODO
        }
    }

    //connect widgets to ports
    for (int i = 3; i < p_n_ports; i++) {
        slot<void> slot1 = compose(bind<0>(mem_fun(*this, &rogueGUI::write_control), i),
            mem_fun(*scales[i], &Changeable::get_value));
        scales[i]->connect(slot1);
        if (p_port_meta[i].type == KNOB) {
            slot<void> slot2 = compose(bind<0>(mem_fun(*this, &rogueGUI::change_status_bar), i),
                mem_fun(*scales[i], &Changeable::get_value));
            scales[i]->connect(slot2);
        }
    }

    HBox* oscs = manage(new HBox());
    for (int i = 0; i < NOSC; i++) {
        oscs->pack_start(createOSC(i));
    }
    HBox* filters = manage(new HBox());
    for (int i = 0; i < NDCF; i++) {
        filters.pack_start(createFilter(i));
    }

    // TODO tabs (presets, lfos, envelopes, modulation, effects)
    // TODO main controls

    /*HBox* header = manage(new HBox());
    header->pack_start(*manage(new Image("analogue.png")));
    header->pack_end(*scales[p_amp_output - 3]->get_widget());
    header->set_border_width(5);
    mainBox.pack_start(*align(header));*/

    mainBox.pack_start(*align(oscs));
    mainBox.pack_start(*align(filters));
    mainBox.pack_end(statusbar);

    add(*align(&mainBox));

    std::cout << "GUI ready" <<std::endl;
}

// FIXME
Widget* rogueGUI::createOSC(int i) {
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

// FIXME
Widget* rogueGUI::createFilter(int i) {
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

// FIXME
Widget* rogueGUI::createLFO(int i) {
    Table* table = manage(new Table(2, 5));
    control(table, "Type", p_lfo1_type, 0, 1);
    control(table, "Freq", p_lfo1_freq, 1, 1);
    control(table, "Delay", p_lfo1_delay, 2, 1);
    control(table, "Fade In", p_lfo1_fade_in, 3, 1);
    control(table, "Width", p_lfo1_width, 4, 1);
    return frame("LFO1", p_lfo1_power, table);
}

// FIXME
Widget* rogueGUI::createEnv(int i) {
    // TODO
}

void rogueGUI::control(Table* table, const char* label, int port_index, int left, int top) {
    table->attach(*scales[port_index]->get_widget(), left, left + 1, top, top + 1);
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

    Panel* panel = manage(new Panel(label, scales[toggle]->get_widget(), content));

    Alignment* alignment = manage(new Alignment(0.0, 0.0, 1.0, 0.0));
    alignment->add(*panel);
    return alignment;
}

Alignment* rogueGUI::align(Widget* widget) {
    Alignment* alignment = manage(new Alignment(0.0, 0.0, 0.0, 0.0));
    alignment->add(*widget);
    return alignment;
}

void rogueGUI::port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer) {
    if (port > 2) {
        scales[port]->set_value(*static_cast<const float*>(buffer));
    }
}

void rogueGUI::change_status_bar(uint32_t port, float value) {
   if (p_port_meta[port-3].step >= 1.0f) {
       sprintf(statusBarText, "%s = %3.0f", p_port_meta[port].symbol, value);
   } else {
       sprintf(statusBarText, "%s = %3.3f", p_port_meta[port].symbol, value);
   }
   statusbar.remove_all_messages();
   statusbar.push(statusBarText);
}

static int _ = rogueGUI::register_class("http://www.github.com/timowest/rogue/gui");

