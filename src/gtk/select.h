
/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef COMBO_H
#define COMBO_H

#include <gtkmm.h>
#include <iostream>
#include <stdio.h>

#include "gui/changeable.h"

namespace rogue {

class SelectBox : public Gtk::EventBox, public Changeable {
  public:
    SelectBox(const char** labels, int count);
    bool on_button_press_event(GdkEventButton* event);
    void on_menu_selection(int i);
    float get_value();
    void set_value(float val);
    Gtk::Widget* get_widget();
    void connect(sigc::slot<void> s);

  private:
    const char** labels;
    int count;
    int index;
    Gtk::Menu menuPopup;
    Gtk::EventBox eventBox;
    Gtk::Label label;
    sigc::signal<void> value_changed;
};

// implementation

SelectBox::SelectBox(const char** labels, int count) : labels(labels), count(count){
    add(label);
    // menu shown on button press
    signal_button_press_event().connect(sigc::mem_fun(*this, &SelectBox::on_button_press_event) );

    // menu creation
    for (int i = 0; i < count; i++) {
        Gtk::MenuItem* menuItem = manage(new Gtk::MenuItem(labels[i]));
        menuItem->signal_activate().connect(
            sigc::bind(sigc::mem_fun(*this, &SelectBox::on_menu_selection), i));
        menuPopup.append(*menuItem);
    }
    menuPopup.show_all();
    set_value(0.0f);
}

bool SelectBox::on_button_press_event(GdkEventButton* event) {
    if(event->type == GDK_BUTTON_PRESS && event->button == 1) {
        menuPopup.popup(event->button, event->time);
        return true;
    } else {
        return false;
    }
}

void SelectBox::on_menu_selection(int i) {
    index = i;
    label.set_text(labels[index]);
    value_changed.emit();
}

float SelectBox::get_value() {
    return (float)index;
}

void SelectBox::set_value(float val) {
    index = (int)val;
    label.set_text(labels[index]);
}

Gtk::Widget* SelectBox::get_widget() {
    return this;
}

void SelectBox::connect(sigc::slot<void> s) {
    value_changed.connect(s);
}

}

#endif //COMBO_H
