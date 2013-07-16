#include <QApplication>
#include <QComboBox>
#include <QDial>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalMapper>
#include <QString>
#include <QTabWidget>

#include "common.h"
#include "rogue.gen"
#include "gui/config.gen"
#include "qt/texts.h"
#include "qt/widgets.h"

class rogueGui : public QWidget {

    Q_OBJECT

    Widget* widgets[p_n_ports];

    QSignalMapper mapper;

    QDial* createDial(int p, bool big = false) {
        int size = big ? 45 : 35;
        const port_meta_t& port = p_port_meta[p];
        CustomDial* dial = new CustomDial(port.min, port.max, port.step, port.default_value);
        dial->setFixedSize(size, size);
        mapper.setMapping(dial, p);
        connect(dial, SIGNAL(valueChanged(int)), &mapper, SLOT(map()));
        widgets[p] = dial;
        return dial;
    }

    QRadioButton* createToggle(int p) {
        CustomRadioButton* button = new CustomRadioButton();
        button->setChecked(p_port_meta[p].default_value > 0.0);
        mapper.setMapping(button, p);
        connect(button, SIGNAL(toggled(bool)), &mapper, SLOT(map()));
        widgets[p] = button;
        return button;
    }

    QPushButton* createToggle(int p, const char* label) {
        CustomPushButton* button = new CustomPushButton();
        button->setText(label);
        button->setCheckable(true);
        button->setChecked(p_port_meta[p].default_value > 0.0);
        mapper.setMapping(button, p);
        connect(button, SIGNAL(toggled(bool)), &mapper, SLOT(map()));
        widgets[p] = button;
        return button;
    }

    QComboBox* createSelect(int p, const char** texts, int size) {
        CustomComboBox* box = new CustomComboBox();
        for (int i = 0; i < size; i++) {
            box->addItem(texts[i]);
        }
        mapper.setMapping(box, p);
        connect(box, SIGNAL(currentIndexChanged(int)), &mapper, SLOT(map()));
        widgets[p] = box;
        return box;
    }

    void connectBox(int p, QGroupBox* box) {
        mapper.setMapping(box, p);
        connect(box, SIGNAL(toggled(bool)), &mapper, SLOT(map()));
        widgets[p] = new GroupBoxAdapter(box);
    }

    QWidget* createBrowser(QWidget* parent) {
        return parent;
    }

    QWidget* createMain(QWidget* parent) {
        parent->setObjectName("main");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(p_volume), 0, 0);
        grid->addWidget(createDial(p_bus_a_level), 0, 1);
        grid->addWidget(createDial(p_bus_a_pan), 0, 2);
        grid->addWidget(createDial(p_bus_b_level), 0, 3);
        grid->addWidget(createDial(p_bus_b_pan), 0, 4);
        // row 2
        grid->addWidget(new QLabel("Vol"),   1, 0);
        grid->addWidget(new QLabel("Vol A"), 1, 1);
        grid->addWidget(new QLabel("Pan A"), 1, 2);
        grid->addWidget(new QLabel("Vol B"), 1, 3);
        grid->addWidget(new QLabel("Pan B"), 1, 4);
        grid->setColumnStretch(5, 1);
        grid->setRowStretch(2, 1);
        return parent;
    }

    QWidget* createChorus(QGroupBox* parent) {
        parent->setObjectName("chorus");
        parent->setCheckable(true);
        connectBox(p_chorus_on, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(p_chorus_t), 0, 0);
        grid->addWidget(createDial(p_chorus_width), 0, 1);
        grid->addWidget(createDial(p_chorus_rate), 0, 2);
        grid->addWidget(createDial(p_chorus_blend), 0, 3);
        grid->addWidget(createDial(p_chorus_feedforward), 0, 4);
        grid->addWidget(createDial(p_chorus_feedback), 0, 5);
        // row 2
        grid->addWidget(new QLabel("T"), 1, 0);
        grid->addWidget(new QLabel("Width"), 1, 1);
        grid->addWidget(new QLabel("Rate"), 1, 2);
        grid->addWidget(new QLabel("Blend"), 1, 3);
        grid->addWidget(new QLabel("Feedforward"), 1, 4);
        grid->addWidget(new QLabel("Feedback"), 1, 5);
        grid->setColumnStretch(6, 1);
        return parent;
    }

    QWidget* createPhaser(QGroupBox* parent) {
        parent->setObjectName("phaser");
        parent->setCheckable(true);
        connectBox(p_phaser_on, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(p_phaser_rate), 0, 0);
        grid->addWidget(createDial(p_phaser_depth), 0, 1);
        grid->addWidget(createDial(p_phaser_spread), 0, 2);
        grid->addWidget(createDial(p_phaser_resonance), 0, 3);
        // row 2
        grid->addWidget(new QLabel("Rate"), 1, 0);
        grid->addWidget(new QLabel("Depth"), 1, 1);
        grid->addWidget(new QLabel("Spread"), 1, 2);
        grid->addWidget(new QLabel("Resonance"), 1, 3);
        grid->setColumnStretch(4, 1);
        return parent;
    }

    QWidget* createDelay(QGroupBox* parent) {
        parent->setObjectName("delay");
        parent->setCheckable(true);
        connectBox(p_delay_on, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(p_delay_bpm), 0, 0);
        grid->addWidget(createDial(p_delay_divider), 0, 1);
        grid->addWidget(createDial(p_delay_feedback), 0, 2);
        grid->addWidget(createDial(p_delay_dry), 0, 3);
        grid->addWidget(createDial(p_delay_blend), 0, 4);
        grid->addWidget(createDial(p_delay_tune), 0, 5);
        // row 2
        grid->addWidget(new QLabel("BPM"), 1, 0);
        grid->addWidget(new QLabel("Divider"), 1, 1);
        grid->addWidget(new QLabel("Feedback"), 1, 2);
        grid->addWidget(new QLabel("Dry"), 1, 3);
        grid->addWidget(new QLabel("Blend"), 1, 4);
        grid->addWidget(new QLabel("Tune"), 1, 5);
        grid->setColumnStretch(6, 1);
        return parent;
    }

    QWidget* createReverb(QGroupBox* parent) {
        parent->setObjectName("reverb");
        parent->setCheckable(true);
        connectBox(p_reverb_on, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(p_reverb_bandwidth), 0, 0);
        grid->addWidget(createDial(p_reverb_tail), 0, 1);
        grid->addWidget(createDial(p_reverb_damping), 0, 2);
        grid->addWidget(createDial(p_reverb_blend), 0, 3);
        // row 2
        grid->addWidget(new QLabel("Bandwidth"), 1, 0);
        grid->addWidget(new QLabel("Tail"), 1, 1);
        grid->addWidget(new QLabel("Damping"), 1, 2);
        grid->addWidget(new QLabel("Blend"), 1, 3);
        grid->setColumnStretch(4, 1);
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
        int off = i * OSC_OFF;
        parent->setCheckable(true);
        connectBox(p_osc1_on + off, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createSelect(p_osc1_type + off, osc_types, 34 + 3 + 4), 0, 0, 1, 2);
        grid->addWidget(createToggle(p_osc1_inv + off, "Inv"), 0, 2);
        grid->addWidget(createToggle(p_osc1_tracking + off, "Track"), 0, 3);
        grid->addWidget(createToggle(p_osc1_sync + off, "Sync"), 0, 4);
        // row 2
        grid->addWidget(createDial(p_osc1_coarse + off), 1, 0);
        grid->addWidget(createDial(p_osc1_fine + off), 1, 1);
        grid->addWidget(createDial(p_osc1_ratio + off), 1, 2);
        grid->addWidget(createDial(p_osc1_level + off), 1, 3);
        grid->addWidget(new WaveDisplay(120, 60), 1, 4, 2, 3);
        // row 3
        grid->addWidget(new QLabel("Coarse"), 2, 0);
        grid->addWidget(new QLabel("Fine"), 2, 1);
        grid->addWidget(new QLabel("Ratio"), 2, 2);
        grid->addWidget(new QLabel("Level"), 2, 3);
        // row 4
        grid->addWidget(createDial(p_osc1_tone + off), 3, 0);
        grid->addWidget(createDial(p_osc1_width + off), 3, 1);
        grid->addWidget(createDial(p_osc1_level_a + off), 3, 2);
        grid->addWidget(createDial(p_osc1_level_b + off), 3, 3);
        if (i > 0) {
            grid->addWidget(createSelect(p_osc1_input + off, nums, i), 3, 4);
            grid->addWidget(createSelect(p_osc1_out_mod + off, out_mod, 4), 3, 5);
            grid->addWidget(createDial(p_osc1_pm + off), 3, 6);
        }
        // row 5
        grid->addWidget(new QLabel("Tone"), 4, 0);
        grid->addWidget(new QLabel("Width"), 4, 1);
        grid->addWidget(new QLabel("Vol A"), 4, 2);
        grid->addWidget(new QLabel("Vol B"), 4, 3);
        if (i > 0) {
            grid->addWidget(new QLabel("Input"), 4, 4);
            grid->addWidget(new QLabel("Mod"), 4, 5);
            grid->addWidget(new QLabel("PM"), 4, 6);
        }
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
        int off = i * DCF_OFF;
        parent->setCheckable(true);
        connectBox(p_filter1_on + off, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createSelect(p_filter1_type + off, filter_types, 12), 0, 0, 1, 2);
        grid->addWidget(createSelect(p_filter1_source + off, filter_sources, 2 + i), 0, 2, 1, 2);
        // row 2
        grid->addWidget(createDial(p_filter1_freq + off), 1, 0);
        grid->addWidget(createDial(p_filter1_q + off), 1, 1);
        grid->addWidget(createDial(p_filter1_level + off), 1, 2);
        grid->addWidget(createDial(p_filter1_pan + off), 1, 3);
        grid->addWidget(new WaveDisplay(120, 60), 1, 4, 2, 3);
        // row 3
        grid->addWidget(new QLabel("Freq"), 2, 0);
        grid->addWidget(new QLabel("Res"), 2, 1);
        grid->addWidget(new QLabel("Vol"), 2, 2);
        grid->addWidget(new QLabel("Pan"), 2, 3);
        // row 4
        grid->addWidget(createDial(p_filter1_distortion + off), 3, 0);
        grid->addWidget(createDial(p_filter1_key_to_f + off), 3, 1);
        grid->addWidget(createDial(p_filter1_vel_to_f + off), 3, 2);
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
        int off = i * ENV_OFF;
        parent->setCheckable(true);
        connectBox(p_env1_on + off, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(p_env1_attack + off), 0, 0);
        grid->addWidget(createDial(p_env1_decay + off), 0, 1);
        grid->addWidget(createDial(p_env1_sustain + off), 0, 2);
        grid->addWidget(createDial(p_env1_release + off), 0, 3);
        grid->addWidget(new WaveDisplay(100, 60), 0, 4, 2, 3);
        // row 2
        grid->addWidget(new QLabel("A"), 1, 0);
        grid->addWidget(new QLabel("D"), 1, 1);
        grid->addWidget(new QLabel("S"), 1, 2);
        grid->addWidget(new QLabel("R"), 1, 3);
        // row 3
        grid->addWidget(createDial(p_env1_hold + off), 2, 0);
        grid->addWidget(createDial(p_env1_pre_delay + off), 2, 1);
        grid->addWidget(createDial(p_env1_curve + off), 2, 2);
        grid->addWidget(createToggle(p_env1_retrigger + off), 2, 3);
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

    QWidget* createMod(QWidget* parent, int i) {
        int off = i * (p_mod11_src - p_mod1_src);
        QGridLayout* grid = new QGridLayout(parent);
        for (int j = 0; j < 5; j++) {
            // col 1
            grid->addWidget(createSelect(p_mod1_src + off, mod_src_labels, M_SIZE), j, 0);
            grid->addWidget(createSelect(p_mod1_target + off, mod_target_labels, M_TARGET_SIZE), j, 1);
            grid->addWidget(new QLabel("c"), j, 2); // TODO control
            // col 2
            grid->addWidget(createSelect(p_mod1_src + off, mod_src_labels, M_SIZE), j, 3);
            grid->addWidget(createSelect(p_mod1_target + off, mod_target_labels, M_TARGET_SIZE), j, 4);
            grid->addWidget(new QLabel("c"), j, 5); // TODO control
            off += 3;
        }
        return parent;
    }

    QWidget* createMod(QWidget* parent) {
        parent->setObjectName("mod");
        QHBoxLayout* layout = new QHBoxLayout();
        QTabWidget* tabs = new QTabWidget();
        tabs->setTabPosition(QTabWidget::West);
        tabs->addTab(createMod(new QGroupBox(), 0), "1");
        tabs->addTab(createMod(new QGroupBox(), 1), "2");
        layout->addWidget(tabs);
        parent->setLayout(layout);
        return parent;
    }

    QWidget* createLfo(QGroupBox* parent, int i) {
        int off = i * LFO_OFF;
        parent->setCheckable(true);
        connectBox(p_lfo1_on + off, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createSelect(p_lfo1_type + off, lfo_types, 5), 0, 0, 1, 2);
        grid->addWidget(createSelect(p_lfo1_reset_type + off, lfo_reset_types, 3), 0, 2, 1, 2);
        // row 2
        grid->addWidget(createDial(p_lfo1_freq + off), 1, 0);
        grid->addWidget(createDial(p_lfo1_width + off), 1, 1);
        grid->addWidget(createDial(p_lfo1_humanize + off), 1, 2);
        grid->addWidget(new WaveDisplay(100, 60), 1, 3, 2, 3);
        // row 3
        grid->addWidget(new QLabel("Freq"), 2, 0);
        grid->addWidget(new QLabel("Width"), 2, 1);
        grid->addWidget(new QLabel("Rand"), 2, 2);
        grid->setRowStretch(3, 1);
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

    // host to UI
    void port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer) {
        if (port > 2) {
            widgets[port]->set_value(*static_cast<const float*>(buffer));
        }
    }

    // UI to host
    Q_SLOT void portChange(int p) {
        //writeControl(p, widgets[p]->get_value());
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

        connect(&mapper, SIGNAL(mapped(int)), SLOT(portChange(int)));
    }

    ~rogueGui() {
        for (int i = 0; i < p_n_ports; i++) {
            delete widgets[i];
        }
    }
};

#include "qt/rogue-gui.mcpp"
