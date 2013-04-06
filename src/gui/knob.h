/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef KNOB_H
#define KNOB_H

// TODO : optimize imports
#include <gtkmm.h>
#include <gdkmm/general.h>  // for cairo helper functions
#include <iostream>
#include <cstring>

#include <iostream>
#include <stdio.h>

#include "gui/changeable.h"

using namespace Cairo;

namespace rogue {

class Knob : public Gtk::DrawingArea, public Changeable {

  public:
    Knob(float min, float max, float step);
    Knob(float min, float max);
    bool on_motion_notify(GdkEventMotion* event);
    bool on_expose_event(GdkEventExpose* event);
    bool on_button_press(GdkEventButton* event);
    float get_value() { return value; }
    void set_value(float val) { value = val; }
    void set_radius(float r) { radius = r; }
    void set_line_width(float w) { line_width = w; }
    void set_size(int s);
    void refresh();
    sigc::signal<void> signal_value_changed();
    Gtk::Widget* get_widget();
    void connect(sigc::slot<void> slot);

  protected:
    float value;
    float min, max, step;
    float line_width = 2.5;
    float radius = 15.0;
    float range, sensitivity, origin_val, origin_y;
    sigc::signal<void> value_changed;
};

// implementation

Knob::Knob(float min, float max, float step) : value(0.0), min(min), max(max), step(step) {
    set_size_request(40, 40);
    range = max - min;
    sensitivity = range / step;
    //sensitivity = range / 100.0;

    add_events( Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON1_MOTION_MASK);
    signal_motion_notify_event().connect(mem_fun(this, &Knob::on_motion_notify));
    signal_button_press_event().connect(mem_fun(this, &Knob::on_button_press));
}

Knob::Knob(float min, float max) : Knob(min, max, (max-min) / 100.0) {}

bool Knob::on_motion_notify(GdkEventMotion* event) {
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

bool Knob::on_expose_event(GdkEventExpose* event) {
    static Gdk::Color bgColor = Gdk::Color("black");
    static Gdk::Color activeColor = Gdk::Color("black");
    static Gdk::Color passiveColor = Gdk::Color("white");
    Glib::RefPtr<Gdk::Window> window = get_window();

    if (window) {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

        Gtk::Allocation allocation = get_allocation();
        const int width = allocation.get_width();
        const int height = allocation.get_height();

        int xc = width / 2;
        int yc = height / 2;
        double val = (value - min) / (max - min);

        double angle = (0.75 + val * 1.5) * M_PI;
        //double angle_start = 0.75 * M_PI;
        //double angle_end = 2.25  * M_PI;

        cr->set_antialias(ANTIALIAS_SUBPIXEL);

        // radial gradient
        cr->save();
        Cairo::RefPtr<Cairo::RadialGradient> radial = Cairo::RadialGradient::create(xc, yc, radius, xc - 2.0, yc - 2.0, radius);
        radial->add_color_stop_rgba(0,  0.0, 0.0, 0.0, 0.5);
        radial->add_color_stop_rgba(1,  0.0, 0.0, 0.0, 0.0);
        cr->set_source(radial);
        cr->arc(xc, yc, radius * 1.2, 0.0, 2 * M_PI);
        cr->fill();
        cr->restore();

        // arc
        cr->save();
        Gdk::Cairo::set_source_color(cr, bgColor);
        cr->arc(xc, yc, radius, 0, 2.0 * M_PI);
        cr->set_line_width(line_width);
        cr->stroke();
        cr->restore();

        // line
        cr->save();
        cr->move_to(xc + 0.3 * radius * cos(angle), yc + 0.3 * radius * sin(angle));
        cr->line_to(xc + 0.9 * radius * cos(angle), yc + 0.9 * radius * sin(angle));
        cr->set_line_width(line_width);
        cr->stroke();
        cr->restore();
    }

    return true;
}

bool Knob::on_button_press(GdkEventButton* event) {
    origin_val = value;
    origin_y = event->y;
    return true;
}

void Knob::set_size(int s) {
    set_size_request(s, s);
}

void Knob::refresh() {
    Glib::RefPtr<Gdk::Window> win = get_window();
    if (win) {
        Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
        win->invalidate_rect(r, false);
    }
}

sigc::signal<void> Knob::signal_value_changed() {
    return value_changed;
}

Gtk::Widget* Knob::get_widget() {
    return this;
}

void Knob::connect(sigc::slot<void> slot) {
    value_changed.connect( slot );
}

}

#endif //KNOB_H
