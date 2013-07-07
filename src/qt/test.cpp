#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGroupBox browser("Browser"), main("Main"), effects("Effects");
    QGroupBox oscs("Oscillators"), filters("Filters");
    QGroupBox envs("Envelopes"), mod("Modulation"), lfos("LFOs");

    // oscillators
    QGroupBox osc1("1"), osc2("2"), osc3("3"), osc4("4");
    QGridLayout oscGrid(&oscs);
    oscGrid.addWidget(&osc1, 1, 1);
    oscGrid.addWidget(&osc2, 1, 2);
    oscGrid.addWidget(&osc3, 2, 1);
    oscGrid.addWidget(&osc4, 2, 2);

    // filters
    QGroupBox filter1("1"), filter2("2");
    QGridLayout filterGrid(&filters);
    filterGrid.addWidget(&filter1, 1, 1);
    filterGrid.addWidget(&filter2, 2, 1);

    // tabs
    QTabWidget effectsTabs(&effects);
    QTabWidget envsTabs(&envs);
    QTabWidget modTabs(&mod);
    QTabWidget lfosTabs(&lfos);

    // main grid
    QWidget window;
    QGridLayout mainGrid(&window);
    mainGrid.addWidget(&browser, 1, 1);
    mainGrid.addWidget(&main, 1, 2);
    mainGrid.addWidget(&effects, 1, 3);
    mainGrid.addWidget(&oscs, 2, 1, 2, 2);
    mainGrid.addWidget(&filters, 2, 3, 2, 1);
    mainGrid.addWidget(&envs, 4, 1);
    mainGrid.addWidget(&mod, 4, 2);
    mainGrid.addWidget(&lfos, 4, 3);

    window.resize(900, 500);
    window.setWindowTitle("rogue");
    window.show();
    return app.exec();
}
