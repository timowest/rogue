#define ROGUI_UI_TEST

#include "lfo.cpp"
#include "tables.cpp"
#include "filter.cpp"
#include "oscillator.cpp"
#include "envelope.cpp"
#include "gui/rogue-gui.cpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    rogueGUI gui("http://www.github.com/timowest/rogue/gui");
    //window.resize(900, 500);
    gui.container().setWindowTitle("rogue");
    gui.container().show();
    return app.exec();
}
