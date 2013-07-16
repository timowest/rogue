/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef CHANGEABLE_H
#define CHANGEABLE_H

#include <gtkmm.h>
#include <iostream>

namespace rogue {

class Changeable {
  public:
    virtual float get_value() = 0;
    virtual void set_value(float val) = 0;
    virtual void connect(sigc::slot<void> slot) = 0;
    virtual Gtk::Widget* get_widget() = 0;
};

}

#endif //CHANGEABLE_H
