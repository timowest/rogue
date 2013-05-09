/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef WAVEDRAW_H
#define WAVEDRAW_H

// TODO : optimize imports
#include <gtkmm.h>
#include <gdkmm/general.h>  // for cairo helper functions
#include <iostream>
#include <cstring>

#include <iostream>
#include <stdio.h>

using namespace Cairo;

namespace rogue {

class Wavedraw : public Gtk::DrawingArea {

  public:
    Wavedraw(int w, int h);
    bool on_expose_event(GdkEventExpose* event);
    void set_contents(float* samples, int size);

  protected:
    float* samples;
    int size = 0;
};

// implementation

Wavedraw::Wavedraw(int w, int h) {
    set_size_request(w, h);
    add_events(Gdk::EXPOSURE_MASK);
}

void Wavedraw::set_contents(float* samples, int size) {
    this->samples = samples;
    this->size = size;
}

bool Wavedraw::on_expose_event(GdkEventExpose* event) {
    static Gdk::Color lineColor = Gdk::Color("#808080");
    Glib::RefPtr<Gdk::Window> window = get_window();

    if (window) {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

        Gtk::Allocation allocation = get_allocation();
        const int width = allocation.get_width();
        const int height = allocation.get_height();

        cr->set_antialias(ANTIALIAS_SUBPIXEL);

        // rectangle
        cr->save();
        Gdk::Cairo::set_source_color(cr, lineColor);
        cr->rectangle(0, 0, width, height);
        cr->stroke();
        cr->restore();

        // line
        cr->save();
        Gdk::Cairo::set_source_color(cr, lineColor);
        for (int i = 0; i < size; i++) {
            float x = i * width / float(size);
            float y = height * 0.5 * (-samples[i] + 1.0);
            if (i > 0) {
                cr->line_to(x, y);
            }
            cr->move_to(x, y);
        }
        cr->stroke();
        cr->restore();
    }

    return true;
}

}

#endif //WAVEDRAW_H

