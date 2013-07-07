#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QFile>
#include <QString>

// browser

// main

// effects

void createOscillators(QWidget* parent) {
	QGridLayout* oscGrid = new QGridLayout(parent);
	oscGrid->addWidget(new QGroupBox("1"), 1, 1);
	oscGrid->addWidget(new QGroupBox("2"), 1, 2);
	oscGrid->addWidget(new QGroupBox("3"), 2, 1);
	oscGrid->addWidget(new QGroupBox("4"), 2, 2);
}

void createFilters(QWidget* parent) {
	QGridLayout* filterGrid = new QGridLayout(parent);
	filterGrid->addWidget(new QGroupBox("1"), 1, 1);
	filterGrid->addWidget(new QGroupBox("2"), 2, 1);
}

// envs

// mod

// lfos

int main(int argc, char *argv[]) {
	QFile file("src/qt/stylesheet.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
    QApplication app(argc, argv);
    app.setStyleSheet(styleSheet);

    QGroupBox browser("Browser"), main("Main"), effects("Effects");
    QGroupBox oscs("Oscillators"), filters("Filters");
    QGroupBox envs("Envelopes"), mod("Modulation"), lfos("LFOs");

    createOscillators(&oscs);
    createFilters(&filters);

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

