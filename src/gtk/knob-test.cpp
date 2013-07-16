/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <gtkmm/main.h>
#include "gui/knob.h"

int main(int argc, char* argv[]) {
    Gtk::Main kit(argc, argv);

    rogue::Knob knob(0.0, 1.0, 0.01);
    knob.set_radius(12.0);
    knob.set_size(38);
    Gtk::Alignment alignment(0.0, 0.0, 0.0, 0.0);
    alignment.add(knob);

    rogue::Knob knob2(-1.0, 1.0, 0.01);
    knob2.set_radius(12.0);
    knob2.set_size(38);
    Gtk::Alignment alignment2(0.0, 0.0, 0.0, 0.0);
    alignment2.add(knob2);

    Gtk::HBox mainBox;
    mainBox.pack_start(alignment);
    mainBox.pack_start(alignment2);

    Gtk::Window window;
    window.set_title("Knob");
    window.set_default_size(800, 400);
    window.add(mainBox);
    window.show_all();

    Gtk::Main::run(window);

    return 0;
}
