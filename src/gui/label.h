/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef LABEL_H
#define LABEL_H

#include <gtkmm.h>
#include <iostream>

#include "gui/changeable.h"

using namespace Gtk;

namespace rogue {

class LabelBox : public Gtk::EventBox, public Changeable {

  public:
    LabelBox(float min, float max, float step);
    bool on_motion_notify(GdkEventMotion* event);
    bool on_scroll_event(GdkEventScroll* event);
    bool on_button_press(GdkEventButton* event);
    float get_value() { return value; }
    void set_value(float val) { value = val; }
    void refresh();
    sigc::signal<void> signal_value_changed();
    Gtk::Widget* get_widget();
    void connect(sigc::slot<void> slot);

  private:
    float value;
    float min, max, step;
    float range, sensitivity, origin_val, origin_y;
    Gtk::Label label;
    sigc::signal<void> value_changed;
    char labelText[10];
};

LabelBox::LabelBox(float min, float max, float step) : value(0.0), min(min), max(max), step(step) {
    add(label);
    range = max - min;
    sensitivity = range / step;

    add_events( Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON1_MOTION_MASK);
    signal_motion_notify_event().connect(mem_fun(this, &LabelBox::on_motion_notify));
    signal_button_press_event().connect(mem_fun(this, &LabelBox::on_button_press));
    signal_scroll_event().connect(mem_fun(this, &LabelBox::on_scroll_event));
    refresh();
}

bool LabelBox::on_motion_notify(GdkEventMotion* event) {
    float offset = (origin_y - event->y) * range / sensitivity;
    float new_value = origin_val + ((step == 0.0) ? offset : step * floor ((offset / step) + 0.5));
    if (new_value > max) {
        new_value = max;
    } else if (new_value < min) {
        new_value = min;
    }
    value = new_value;
    value_changed.emit();
    refresh();
    return true;
}

bool LabelBox::on_scroll_event(GdkEventScroll* event) {
    float new_value;
    if (event->direction == GDK_SCROLL_UP) {
        new_value = value + step;
        if (new_value > max) {
            new_value = max;
        }
    } else if (event->direction == GDK_SCROLL_DOWN) {
        new_value = value - step;
        if (new_value < min) {
            new_value = min;
        }
    }
    value = new_value;
    value_changed.emit();
    refresh();
    return true;
}

bool LabelBox::on_button_press(GdkEventButton* event) {
    origin_val = value;
    origin_y = event->y;
    return true;
}

void LabelBox::refresh() {
    sprintf(labelText, "%3.2f", value);
    label.set_text(labelText);
}

sigc::signal<void> LabelBox::signal_value_changed() {
    return value_changed;
}

Gtk::Widget* LabelBox::get_widget() {
    return this;
}

void LabelBox::connect(sigc::slot<void> slot) {
    value_changed.connect( slot );
}

}

#endif //LABEL_H
