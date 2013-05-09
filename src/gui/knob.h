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
    bool on_motion_notify(GdkEventMotion* event);
    bool on_scroll_event(GdkEventScroll* event);
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
    float val0, angle0;
    float line_width = 2.0;
    float radius = 15.0;
    float range, sensitivity, origin_val, origin_y;
    sigc::signal<void> value_changed;
};

// implementation

Knob::Knob(float min, float max, float step) : value(0.0), min(min), max(max), step(step) {
    set_size_request(40, 40);
    range = max - min;
    sensitivity = range / step;

    val0 = -min / (max - min);
    angle0 = (0.75 + val0 * 1.5) * M_PI;

    add_events( Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON1_MOTION_MASK);
    signal_motion_notify_event().connect(mem_fun(this, &Knob::on_motion_notify));
    signal_button_press_event().connect(mem_fun(this, &Knob::on_button_press));
    signal_scroll_event().connect(mem_fun(this, &Knob::on_scroll_event));
}

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

bool Knob::on_scroll_event(GdkEventScroll* event) {
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

bool Knob::on_expose_event(GdkEventExpose* event) {
    static Gdk::Color bgColor = Gdk::Color("#909090");
    static Gdk::Color fgColor = Gdk::Color("#c0c0c0");
    static Gdk::Color fg2Color = Gdk::Color("#b0b0b0");
    static Gdk::Color arcColor = Gdk::Color("#404040");
    static Gdk::Color lineColor = Gdk::Color("#ffffff");
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

        cr->set_antialias(ANTIALIAS_SUBPIXEL);

        // background
        cr->save();
        Gdk::Cairo::set_source_color(cr, bgColor);
        cr->arc(xc, yc + radius * 0.2, radius * 1.2, 0, 2.0 * M_PI);
        cr->fill();
        cr->restore();

        // foreground
        cr->save();
        Gdk::Cairo::set_source_color(cr, fgColor);
        cr->arc(xc, yc, radius, 0, 2.0 * M_PI);
        cr->fill();
        cr->restore();

        // full arc
        cr->save();
        Gdk::Cairo::set_source_color(cr, fg2Color);
        cr->arc(xc, yc, radius, 0, 2.0 * M_PI);
        cr->set_line_width(1.5);
        cr->stroke();
        cr->restore();

        // arc
        cr->save();
        Gdk::Cairo::set_source_color(cr, arcColor);
        if (angle0 < angle) {
            cr->arc(xc, yc, radius + 5.0, angle0, angle);
        } else {
            cr->arc(xc, yc, radius + 5.0, angle, angle0);
        }
        cr->set_line_width(3.0);
        cr->stroke();
        cr->restore();

        // line
        cr->save();
        Gdk::Cairo::set_source_color(cr, lineColor);
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
    Glib::RefPtr<Gdk::Window> window = get_window();

    if (window) {
        Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
        window->invalidate_rect(r, false);
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

