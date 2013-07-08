#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QFile>
#include <QString>

class rogueGui : public QWidget {

	// browser

	// main

	QWidget* createEffects(QWidget* parent) {
		QHBoxLayout* layout = new QHBoxLayout();
		QTabWidget* tabs = new QTabWidget();
		tabs->setTabPosition(QTabWidget::West);
		tabs->addTab(new QGroupBox(), "C");
		tabs->addTab(new QGroupBox(), "P");
		tabs->addTab(new QGroupBox(), "D");
		tabs->addTab(new QGroupBox(), "R");
		layout->addWidget(tabs);
		parent->setLayout(layout);
		return parent;
	}

	QWidget* createOscillators(QWidget* parent) {
		QGridLayout* oscGrid = new QGridLayout();
		oscGrid->addWidget(new QGroupBox("1"), 1, 1);
		oscGrid->addWidget(new QGroupBox("2"), 1, 2);
		oscGrid->addWidget(new QGroupBox("3"), 2, 1);
		oscGrid->addWidget(new QGroupBox("4"), 2, 2);
		parent->setLayout(oscGrid);
		return parent;
	}

	QWidget* createFilters(QWidget* parent) {
		QGridLayout* filterGrid = new QGridLayout();
		filterGrid->addWidget(new QGroupBox("1"), 1, 1);
		filterGrid->addWidget(new QGroupBox("2"), 2, 1);
		parent->setLayout(filterGrid);
		return parent;
	}

	QWidget* createEnvs(QWidget* parent) {
		QHBoxLayout* layout = new QHBoxLayout();
		QTabWidget* tabs = new QTabWidget();
		tabs->setTabPosition(QTabWidget::West);
		tabs->addTab(new QGroupBox(), "1");
		tabs->addTab(new QGroupBox(), "2");
		tabs->addTab(new QGroupBox(), "3");
		tabs->addTab(new QGroupBox(), "4");
		tabs->addTab(new QGroupBox(), "5");
		layout->addWidget(tabs);
	    parent->setLayout(layout);
	    return parent;
	}

	QWidget* createMod(QWidget* parent) {
		QHBoxLayout* layout = new QHBoxLayout();
		QTabWidget* tabs = new QTabWidget();
		tabs->setTabPosition(QTabWidget::West);
		tabs->addTab(new QGroupBox(), "1");
		tabs->addTab(new QGroupBox(), "2");
		layout->addWidget(tabs);
		parent->setLayout(layout);
		return parent;
	}

	QWidget* createLfos(QWidget* parent) {
		QHBoxLayout* layout = new QHBoxLayout();
		QTabWidget* tabs = new QTabWidget();
		tabs->setTabPosition(QTabWidget::West);
		tabs->addTab(new QGroupBox(), "1");
		tabs->addTab(new QGroupBox(), "2");
		tabs->addTab(new QGroupBox(), "3");
		layout->addWidget(tabs);
		parent->setLayout(layout);
		return parent;
	}

  public:

	rogueGui(QWidget* parent = 0) : QWidget(parent) {
		QWidget* browser = new QGroupBox("Browser");
		QWidget* main = new QGroupBox("Main");
	    QWidget* effects = createEffects(new QGroupBox("Effects"));
	    QWidget* oscs = createOscillators(new QGroupBox("Oscillators"));
	    QWidget* filters = createFilters(new QGroupBox("Filters"));
	    QWidget* envs = createEnvs(new QGroupBox("Envelops"));
	    QWidget* mod = createMod(new QGroupBox("Modulation"));
	    QWidget* lfos = createLfos(new QGroupBox("LFOs"));

	    // main grid
	    QGridLayout* mainGrid = new QGridLayout(this);
	    mainGrid->addWidget(browser, 1, 1);
	    mainGrid->addWidget(main, 1, 2);
	    mainGrid->addWidget(effects, 1, 3);
	    mainGrid->addWidget(oscs, 2, 1, 2, 2);
	    mainGrid->addWidget(filters, 2, 3, 2, 1);
	    mainGrid->addWidget(envs, 4, 1);
	    mainGrid->addWidget(mod, 4, 2);
	    mainGrid->addWidget(lfos, 4, 3);
	}


};


int main(int argc, char *argv[]) {
	QFile file("src/qt/stylesheet.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
    QApplication app(argc, argv);
    app.setStyleSheet(styleSheet);

    rogueGui window;
    window.resize(900, 500);
    window.setWindowTitle("rogue");
    window.show();
    return app.exec();
}

