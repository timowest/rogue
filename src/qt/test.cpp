#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QDial>
#include <QLabel>
#include <QTabWidget>
#include <QFile>
#include <QString>

class oscDisplay : public QFrame {
  public:
    oscDisplay() {
        //setProperty("wave", true);
        setFixedSize(120, 60);
    }
};

class filterDisplay : public QFrame {
  public:
    filterDisplay() {
        //setProperty("wave", true);
        setFixedSize(120, 60);
    }
};

class lfoDisplay : public QFrame {
  public:
    lfoDisplay() {
        //setProperty("wave", true);
        setFixedSize(100, 60);
    }
};

class envDisplay : public QFrame {
  public:
    envDisplay() {
        //setProperty("wave", true);
        setFixedSize(100, 60);
    }
};

class rogueGui : public QWidget {

    QDial* createDial() {
        QDial* dial = new QDial();
        dial->setFixedSize(35, 35);
        return dial;
    }

    QWidget* createBrowser(QWidget* parent) {
        return parent;
    }

    QWidget* createMain(QWidget* parent) {
        parent->setObjectName("main");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(), 0, 0);
        grid->addWidget(createDial(), 0, 1);
        grid->addWidget(createDial(), 0, 2);
        grid->addWidget(createDial(), 0, 3);
        grid->addWidget(createDial(), 0, 4);
        // row 2
        grid->addWidget(new QLabel("Vol"),   1, 0);
        grid->addWidget(new QLabel("Vol A"), 1, 1);
        grid->addWidget(new QLabel("Pan A"), 1, 2);
        grid->addWidget(new QLabel("Vol B"), 1, 3);
        grid->addWidget(new QLabel("Pan B"), 1, 4);
        return parent;
    }

    QWidget* createChorus(QGroupBox* parent) {
        parent->setObjectName("chorus");
        parent->setCheckable(true);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(), 0, 0);
        grid->addWidget(createDial(), 0, 1);
        grid->addWidget(createDial(), 0, 2);
        grid->addWidget(createDial(), 0, 3);
        grid->addWidget(createDial(), 0, 4);
        grid->addWidget(createDial(), 0, 5);
        // row 2
        grid->addWidget(new QLabel("T"), 1, 0);
        grid->addWidget(new QLabel("Width"), 1, 1);
        grid->addWidget(new QLabel("Rate"), 1, 2);
        grid->addWidget(new QLabel("Blend"), 1, 3);
        grid->addWidget(new QLabel("Feedforward"), 1, 4);
        grid->addWidget(new QLabel("Feedback"), 1, 5);
        return parent;
    }

    QWidget* createPhaser(QGroupBox* parent) {
        parent->setObjectName("phaser");
        parent->setCheckable(true);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(), 0, 0);
        grid->addWidget(createDial(), 0, 1);
        grid->addWidget(createDial(), 0, 2);
        grid->addWidget(createDial(), 0, 3);
        // row 2
        grid->addWidget(new QLabel("Rate"), 1, 0);
        grid->addWidget(new QLabel("Depth"), 1, 1);
        grid->addWidget(new QLabel("Spread"), 1, 2);
        grid->addWidget(new QLabel("Resonance"), 1, 3);
        return parent;
    }

    QWidget* createDelay(QGroupBox* parent) {
        parent->setObjectName("delay");
        parent->setCheckable(true);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(), 0, 0);
        grid->addWidget(createDial(), 0, 1);
        grid->addWidget(createDial(), 0, 2);
        grid->addWidget(createDial(), 0, 3);
        grid->addWidget(createDial(), 0, 4);
        grid->addWidget(createDial(), 0, 5);
        // row 2
        grid->addWidget(new QLabel("BPM"), 1, 0);
        grid->addWidget(new QLabel("Divider"), 1, 1);
        grid->addWidget(new QLabel("Feedback"), 1, 2);
        grid->addWidget(new QLabel("Dry"), 1, 3);
        grid->addWidget(new QLabel("Blend"), 1, 4);
        grid->addWidget(new QLabel("Tune"), 1, 5);
        return parent;
    }

    QWidget* createReverb(QGroupBox* parent) {
        parent->setObjectName("reverb");
        parent->setCheckable(true);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(), 0, 0);
        grid->addWidget(createDial(), 0, 1);
        grid->addWidget(createDial(), 0, 2);
        grid->addWidget(createDial(), 0, 3);
        // row 2
        grid->addWidget(new QLabel("Bandwidth"), 1, 0);
        grid->addWidget(new QLabel("Tail"), 1, 1);
        grid->addWidget(new QLabel("Damping"), 1, 2);
        grid->addWidget(new QLabel("Blend"), 1, 3);
        return parent;
    }

    QWidget* createEffects(QGroupBox* parent) {
        parent->setObjectName("effects");
        QHBoxLayout* layout = new QHBoxLayout(parent);
        QTabWidget* tabs = new QTabWidget();
        tabs->setTabPosition(QTabWidget::West);
        tabs->addTab(createChorus(new QGroupBox("Chorus")), "C");
        tabs->addTab(createPhaser(new QGroupBox("Phaser")), "P");
        tabs->addTab(createDelay(new QGroupBox("Delay")), "D");
        tabs->addTab(createReverb(new QGroupBox("Reverb")), "R");
        layout->addWidget(tabs);
        return parent;
    }

    QWidget* createOscillator(QGroupBox* parent, int i) {
        parent->setCheckable(true);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(new QLabel("Type"), 0, 0, 1, 2);
        grid->addWidget(new QLabel("Inv"), 0, 2);
        grid->addWidget(new QLabel("Track"), 0, 3);
        // row 2
        grid->addWidget(createDial(), 1, 0);
        grid->addWidget(createDial(), 1, 1);
        grid->addWidget(createDial(), 1, 2);
        grid->addWidget(createDial(), 1, 3);
        grid->addWidget(new oscDisplay(), 1, 4, 2, 3);
        // row 3
        grid->addWidget(new QLabel("Coarse"), 2, 0);
        grid->addWidget(new QLabel("Fine"), 2, 1);
        grid->addWidget(new QLabel("Ratio"), 2, 2);
        grid->addWidget(new QLabel("Level"), 2, 3);
        // row 4
        grid->addWidget(createDial(), 3, 0);
        grid->addWidget(createDial(), 3, 1);
        grid->addWidget(createDial(), 3, 2);
        grid->addWidget(createDial(), 3, 3);
        // row 5
        grid->addWidget(new QLabel("Tone"), 4, 0);
        grid->addWidget(new QLabel("Width"), 4, 1);
        grid->addWidget(new QLabel("Vol A"), 4, 2);
        grid->addWidget(new QLabel("Vol B"), 4, 3);
        return parent;
    }

    QWidget* createOscillators(QWidget* parent) {
        parent->setObjectName("oscillators");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(new QLabel("1"), 0, 0);
        grid->addWidget(createOscillator(new QGroupBox(), 0), 0, 1);
        grid->addWidget(createOscillator(new QGroupBox(), 1), 0, 2);
        grid->addWidget(new QLabel("2"), 0, 3);
        // row 2
        grid->addWidget(new QLabel("3"), 1, 0);
        grid->addWidget(createOscillator(new QGroupBox(), 2), 1, 1);
        grid->addWidget(createOscillator(new QGroupBox(), 3), 1, 2);
        grid->addWidget(new QLabel("4"), 1, 3);
        grid->setColumnStretch(1, 1);
        grid->setColumnStretch(2, 1);
        return parent;
    }

    QWidget* createFilter(QGroupBox* parent, int i) {
        parent->setCheckable(true);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(new QLabel("Type"), 0, 0, 1, 2);
        grid->addWidget(new QLabel("Source"), 0, 2, 1, 2);
        // row 2
        grid->addWidget(createDial(), 1, 0);
        grid->addWidget(createDial(), 1, 1);
        grid->addWidget(createDial(), 1, 2);
        grid->addWidget(createDial(), 1, 3);
        grid->addWidget(new filterDisplay(), 1, 4, 2, 3);
        // row 3
        grid->addWidget(new QLabel("Freq"), 2, 0);
        grid->addWidget(new QLabel("Res"), 2, 1);
        grid->addWidget(new QLabel("Vol"), 2, 2);
        grid->addWidget(new QLabel("Pan"), 2, 3);
        // row 4
        grid->addWidget(createDial(), 3, 0);
        grid->addWidget(createDial(), 3, 1);
        grid->addWidget(createDial(), 3, 2);
        // row 5
        grid->addWidget(new QLabel("Dist"), 4, 0);
        grid->addWidget(new QLabel("K > F"), 4, 1);
        grid->addWidget(new QLabel("V > F"), 4, 2);
        return parent;
    }

    QWidget* createFilters(QWidget* parent) {
        parent->setObjectName("filters");
        QGridLayout* grid = new QGridLayout();
        // row 1
        grid->addWidget(new QLabel("1"), 0, 0);
        grid->addWidget(createFilter(new QGroupBox(), 0), 0, 1);
        // row 2
        grid->addWidget(new QLabel("2"), 1, 0);
        grid->addWidget(createFilter(new QGroupBox(), 1), 1, 1);
        grid->setColumnStretch(1, 1);
        parent->setLayout(grid);
        return parent;
    }

    QWidget* createEnv(QGroupBox* parent, int i) {
        parent->setCheckable(true);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(), 0, 0);
        grid->addWidget(createDial(), 0, 1);
        grid->addWidget(createDial(), 0, 2);
        grid->addWidget(createDial(), 0, 3);
        grid->addWidget(new envDisplay(), 0, 4, 2, 3);
        // row 2
        grid->addWidget(new QLabel("A"), 1, 0);
        grid->addWidget(new QLabel("D"), 1, 1);
        grid->addWidget(new QLabel("S"), 1, 2);
        grid->addWidget(new QLabel("R"), 1, 3);
        // row 3
        grid->addWidget(createDial(), 2, 0);
        grid->addWidget(createDial(), 2, 1);
        grid->addWidget(createDial(), 2, 2);
        grid->addWidget(createDial(), 2, 3);
        // row 4
        grid->addWidget(new QLabel("Hold"), 3, 0);
        grid->addWidget(new QLabel("Pre"), 3, 1);
        grid->addWidget(new QLabel("Curve"), 3, 2);
        grid->addWidget(new QLabel("Retr"), 3, 3);
        return parent;
    }

    QWidget* createEnvs(QWidget* parent) {
        parent->setObjectName("envs");
        QHBoxLayout* layout = new QHBoxLayout(parent);
        QTabWidget* tabs = new QTabWidget();
        tabs->setTabPosition(QTabWidget::West);
        tabs->addTab(createEnv(new QGroupBox(), 0), "1");
        tabs->addTab(createEnv(new QGroupBox(), 1), "2");
        tabs->addTab(createEnv(new QGroupBox(), 2), "3");
        tabs->addTab(createEnv(new QGroupBox(), 3), "4");
        tabs->addTab(createEnv(new QGroupBox(), 4), "5");
        layout->addWidget(tabs);
        return parent;
    }

    QWidget* createMod(QWidget* parent) {
        parent->setObjectName("mod");
        QHBoxLayout* layout = new QHBoxLayout();
        QTabWidget* tabs = new QTabWidget();
        tabs->setTabPosition(QTabWidget::West);
        tabs->addTab(new QGroupBox(), "1");
        tabs->addTab(new QGroupBox(), "2");
        layout->addWidget(tabs);
        parent->setLayout(layout);
        return parent;
    }

    QWidget* createLfo(QGroupBox* parent, int i) {
        parent->setCheckable(true);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(), 0, 0);
        grid->addWidget(createDial(), 0, 1);
        grid->addWidget(createDial(), 0, 2);
        grid->addWidget(createDial(), 0, 3);
        grid->addWidget(new lfoDisplay(), 0, 4, 2, 3);
        // row 2
        grid->addWidget(new QLabel("Type"), 1, 0);
        grid->addWidget(new QLabel("Reset"), 1, 1);
        grid->addWidget(new QLabel("Freq"), 1, 2);
        grid->addWidget(new QLabel("Width"), 1, 3);
        // row 3
        grid->addWidget(createDial(), 2, 0);
        // row 4
        grid->addWidget(new QLabel("Rand"), 3, 0);
        return parent;
    }

    QWidget* createLfos(QWidget* parent) {
        parent->setObjectName("lfos");
        QHBoxLayout* layout = new QHBoxLayout(parent);
        QTabWidget* tabs = new QTabWidget();
        tabs->setTabPosition(QTabWidget::West);
        tabs->addTab(createLfo(new QGroupBox(), 0), "1");
        tabs->addTab(createLfo(new QGroupBox(), 1), "2");
        tabs->addTab(createLfo(new QGroupBox(), 2), "3");
        layout->addWidget(tabs);
        return parent;
    }

  public:

    rogueGui(QWidget* parent = 0) : QWidget(parent) {
        setObjectName("root");
        QWidget* browser = createBrowser(new QGroupBox("Browser"));
        QWidget* main = createMain(new QGroupBox("Main"));
        QWidget* effects = createEffects(new QGroupBox("Effects"));
        QWidget* oscs = createOscillators(new QGroupBox("Oscillators"));
        QWidget* filters = createFilters(new QGroupBox("Filters"));
        QWidget* envs = createEnvs(new QGroupBox("Envelopes"));
        QWidget* mod = createMod(new QGroupBox("Modulation"));
        QWidget* lfos = createLfos(new QGroupBox("LFOs"));

        // main grid
        QGridLayout* grid = new QGridLayout(this);
        grid->addWidget(browser, 0, 0);
        grid->addWidget(main, 0, 1);
        grid->addWidget(effects, 0, 2); // FIXME
        grid->addWidget(oscs, 1, 0, 2, 2);
        grid->addWidget(filters, 1, 2, 2, 1);
        grid->addWidget(envs, 3, 0);
        grid->addWidget(mod, 3, 1);
        grid->addWidget(lfos, 3, 2);
        grid->setRowStretch(1, 1);
        grid->setRowStretch(2, 1);
    }

};


int main(int argc, char *argv[]) {
    QFile file("src/qt/stylesheet.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    QApplication app(argc, argv);
    app.setStyleSheet(styleSheet);

    rogueGui window;
    //window.resize(900, 500);
    window.setWindowTitle("rogue");
    window.show();
    return app.exec();
}

