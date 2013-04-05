/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <gtkmm/main.h>
#include "gui/knob.h"

int main(int argc, char* argv[]) {
    Gtk::Main kit(argc, argv);

    Knob knob(0.0, 1.0, 0.01);
    Gtk::Alignment alignment(0.0, 0.0, 0.0, 0.0);
    alignment.add(knob);

    Gtk::Window window;
    window.set_title("Knob");
    window.set_default_size(800, 400);
    window.add(alignment);
    window.show_all();

    Gtk::Main::run(window);

    return 0;
}
