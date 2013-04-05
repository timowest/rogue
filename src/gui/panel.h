/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef PANEL_H
#define PANEL_H

#include <gtkmm.h>
#include <iostream>

using namespace Gtk;

class Panel : public Frame {
  public:
    Panel(const char* title, Widget* toggle, Widget* content) {
        set_border_width(5);
        set_shadow_type(SHADOW_OUT);
        strcpy(bold_title, "<b>");
        strcat(bold_title, title);
        strcat(bold_title, "</b>");

        Label* label = manage(new Label(bold_title));
        label->set_use_markup();   

        VBox* main = manage(new VBox());
        HBox* header = manage(new HBox());
        header->pack_start(*label, false, false);
        header->pack_end(*toggle, false, false);
        main->set_border_width(2);
        main->pack_start(*header);
        main->pack_start(*content);
        add(*main);
    }

  private:
    char bold_title[40];
};

#endif //PANEL_H
