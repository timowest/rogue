#include <gtkmm/main.h>
#include "lfo.cpp"
#include "tables.cpp"
#include "oscillator.cpp"
#include "gui/rogue-gui.cpp"

int main(int argc, char* argv[]) {
    Gtk::Main kit(argc, argv);

    rogue::rogueGUI guiBox("http://www.github.com/timowest/rogue/gui");

    /*float controls[p_n_ports];
    for (int i = 3; i < p_n_ports; i++) {
        controls[i] = p_port_meta[i].default_value;
    }*/

    Gtk::Window window;
    window.set_title("rogue");
    window.set_default_size(800, 400);
    window.add(guiBox.get_widget());
    window.show_all();

    Gtk::Main::run(window);
    return 0;
}
