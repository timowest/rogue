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

#include <fftw3.h>

#include "common.h"
#include "rogue.gen"
#include "wrappers.h"

#include "qtui.h"
#include "gui/config.gen"
#include "gui/texts.h"
#include "gui/widgets.h"

class rogueGUI : public QObject, public lvtk::UI<rogueGUI, lvtk::QtUI<true>, lvtk::URID<true> > {

    static const int OSC_WIDTH = 120;
    static const int DCF_WIDTH = 120;
    static const int ENV_WIDTH = 120;
    static const int LFO_WIDTH = 120;

    static const int WAVE_HEIGHT = 60;

    Q_OBJECT

    Widget* widgets[p_n_ports];
    QLabel* labels[p_n_ports];

    QSignalMapper mapper, oscMapper, filterMapper, envMapper, lfoMapper;

    rogue::Osc osc;
    rogue::Filter filter;
    dsp::LFO lfo;
    dsp::AHDSR env;

    WaveDisplay* osc_wd[4];
    WaveDisplay* filter_wd[2];
    WaveDisplay* env_wd[4];
    WaveDisplay* lfo_wd[4];

    // fft utils
    float* fftIn[2];
    float* fftOut[2];
    fftwf_plan fftPlan[2];

    QDial* createDial(int p, bool big = false) {
        int size = big ? 40 : 35;
        const port_meta_t& port = p_port_meta[p];
        CustomDial* dial = new CustomDial(port.min, port.max, port.step, port.default_value);
        dial->setFixedSize(size, size);
        mapper.setMapping(dial, p);
        connect(dial, SIGNAL(valueChanged(int)), &mapper, SLOT(map()));
        widgets[p] = dial;
        return dial;
    }

    QLabel* createLabel(int p) {
        QLabel* label = new QLabel();
        label->setNum(p_port_meta[p].default_value);
        label->setProperty("num", QVariant(true));
        labels[p] = label;
        return label;
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

    QDoubleSpinBox* createSpin(int p) {
        const port_meta_t& port = p_port_meta[p];
        CustomSpinBox* spin = new CustomSpinBox();
        spin->setMinimum(port.min);
        spin->setMaximum(port.max);
        spin->setValue(port.default_value);
        spin->setSingleStep(port.step);
        mapper.setMapping(spin, p);
        connect(spin, SIGNAL(valueChanged(double)), &mapper, SLOT(map()));
        widgets[p] = spin;
        return spin;
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

    void connectBox(int p, QGroupBox* box) {
        mapper.setMapping(box, p);
        box->setChecked(p_port_meta[p].default_value > 0.0);
        connect(box, SIGNAL(toggled(bool)), &mapper, SLOT(map()));
        widgets[p] = new GroupBoxAdapter(box);
    }

    QWidget* createIndex(const char* text) {
        QWidget* widget = new QLabel(text);
        widget->setProperty("index", QVariant(true));
        return widget;
    }

    QWidget* createBrowser(QWidget* parent) {
        return parent;
    }

    QWidget* createMain(QWidget* parent) {
        parent->setObjectName("main");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(p_volume), 0, 0);
        grid->addWidget(createSelect(p_play_mode, modes, 3), 0, 1);
        grid->addWidget(createDial(p_glide_time), 0, 2);
        grid->addWidget(createDial(p_pitchbend_range), 0, 3);
        // row 2
        grid->addWidget(new QLabel("Vol"),  1, 0);
        grid->addWidget(new QLabel("Mode"), 1, 1);
        grid->addWidget(new QLabel("Glide t."), 1, 2);
        grid->addWidget(new QLabel("Bend r."), 1, 3);
        // row 3
        grid->addWidget(createLabel(p_volume), 2, 0);
        // skip
        grid->addWidget(createLabel(p_glide_time), 2, 2);
        grid->addWidget(createLabel(p_pitchbend_range), 2, 3);

        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setColumnStretch(4, 1);
        grid->setRowStretch(3, 1);
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
        grid->addWidget(new QLabel("Feedf"), 1, 4);
        grid->addWidget(new QLabel("Feedb"), 1, 5);
        // row 3
        grid->addWidget(createLabel(p_chorus_t), 2, 0);
        grid->addWidget(createLabel(p_chorus_width), 2, 1);
        grid->addWidget(createLabel(p_chorus_rate), 2, 2);
        grid->addWidget(createLabel(p_chorus_blend), 2, 3);
        grid->addWidget(createLabel(p_chorus_feedforward), 2, 4);
        grid->addWidget(createLabel(p_chorus_feedback), 2, 5);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
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
        grid->addWidget(new QLabel("Res"), 1, 3);
        // row 3
        grid->addWidget(createLabel(p_phaser_rate), 2, 0);
        grid->addWidget(createLabel(p_phaser_depth), 2, 1);
        grid->addWidget(createLabel(p_phaser_spread), 2, 2);
        grid->addWidget(createLabel(p_phaser_resonance), 2, 3);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
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
        grid->addWidget(new QLabel("Feedb"), 1, 2);
        grid->addWidget(new QLabel("Dry"), 1, 3);
        grid->addWidget(new QLabel("Blend"), 1, 4);
        grid->addWidget(new QLabel("Tune"), 1, 5);
        // row 3
        grid->addWidget(createLabel(p_delay_bpm), 2, 0);
        grid->addWidget(createLabel(p_delay_divider), 2, 1);
        grid->addWidget(createLabel(p_delay_feedback), 2, 2);
        grid->addWidget(createLabel(p_delay_dry), 2, 3);
        grid->addWidget(createLabel(p_delay_blend), 2, 4);
        grid->addWidget(createLabel(p_delay_tune), 2, 5);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
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
        grid->addWidget(new QLabel("Bandw"), 1, 0);
        grid->addWidget(new QLabel("Tail"), 1, 1);
        grid->addWidget(new QLabel("Damp"), 1, 2);
        grid->addWidget(new QLabel("Blend"), 1, 3);
        // row 4
        grid->addWidget(createLabel(p_reverb_bandwidth), 2, 0);
        grid->addWidget(createLabel(p_reverb_tail), 2, 1);
        grid->addWidget(createLabel(p_reverb_damping), 2, 2);
        grid->addWidget(createLabel(p_reverb_blend), 2, 3);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setColumnStretch(4, 1);
        return parent;
    }

    QWidget* createEffects(QWidget* parent) {
        parent->setObjectName("effects");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createChorus(new QGroupBox("Chorus")), 0, 0);
        grid->addWidget(createPhaser(new QGroupBox("Phaser")), 0, 1);
        grid->addWidget(createDelay(new QGroupBox("Delay")), 0, 2);
        grid->addWidget(createReverb(new QGroupBox("Reverb")), 0, 3);
        grid->setRowStretch(1, 1);
        return parent;
    }

    QDial* connectToOsc(QDial* dial, int i) {
        oscMapper.setMapping(dial, i);
        connect(dial, SIGNAL(valueChanged(int)), &oscMapper, SLOT(map()));
        return dial;
    }

    QPushButton* connectToOsc(QPushButton* button, int i) {
        oscMapper.setMapping(button, i);
        connect(button, SIGNAL(toggled(bool)), &oscMapper, SLOT(map()));
        return button;
    }

    QWidget* createOscillator(QGroupBox* parent, int i) {
        int off = i * OSC_OFF;
        parent->setCheckable(true);
        connectBox(p_osc1_on + off, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        QComboBox* typeBox = createSelect(p_osc1_type + off, osc_types, 34 + 3 + 4);
        oscMapper.setMapping(typeBox, i);
        connect(typeBox, SIGNAL(currentIndexChanged(int)), &oscMapper, SLOT(map()));
        grid->addWidget(typeBox, 0, 0, 1, 2);
        grid->addWidget(connectToOsc(createToggle(p_osc1_inv + off, "Inv"), i), 0, 2);
        grid->addWidget(createToggle(p_osc1_tracking + off, "Track"), 0, 3);
        if (i > 0) {
            grid->addWidget(createToggle(p_osc1_free + off, "Free"), 0, 4);
            grid->addWidget(createToggle(p_osc1_sync + off, "Sync"), 0, 5);
        } else {
            grid->addWidget(createToggle(p_osc1_free + off, "Free"), 0, 4);
        }
        // row 2
        grid->addWidget(osc_wd[i] = new WaveDisplay(OSC_WIDTH, WAVE_HEIGHT), 1, 0, 3, 3);
        grid->addWidget(createDial(p_osc1_coarse + off), 1, 3);
        grid->addWidget(createDial(p_osc1_fine + off), 1, 4);
        grid->addWidget(createDial(p_osc1_ratio + off), 1, 5);
        grid->addWidget(createDial(p_osc1_level + off), 1, 6);
        // row 3
        grid->addWidget(new QLabel("Coarse"), 2, 3);
        grid->addWidget(new QLabel("Fine"), 2, 4);
        grid->addWidget(new QLabel("Ratio"), 2, 5);
        grid->addWidget(new QLabel("Level"), 2, 6);
        // row 4
        grid->addWidget(createLabel(p_osc1_coarse + off), 3, 3);
        grid->addWidget(createLabel(p_osc1_fine + off), 3, 4);
        grid->addWidget(createLabel(p_osc1_ratio + off), 3, 5);
        grid->addWidget(createLabel(p_osc1_level + off), 3, 6);

        // row 5
        grid->addWidget(connectToOsc(createDial(p_osc1_start + off), i), 4, 0);
        grid->addWidget(connectToOsc(createDial(p_osc1_width + off), i), 4, 1);
        if (i > 0) {
            grid->addWidget(createSelect(p_osc1_input + off, nums, i), 4, 4);
            grid->addWidget(createSelect(p_osc1_out_mod + off, out_mod, 4), 4, 5);
            grid->addWidget(createDial(p_osc1_pm + off), 4, 6);
        }
        // row 6
        grid->addWidget(new QLabel("Start"), 5, 0);
        grid->addWidget(new QLabel("Width"), 5, 1);
        if (i > 0) {
            grid->addWidget(new QLabel("Input"), 5, 4);
            grid->addWidget(new QLabel("Mod"), 5, 5);
            grid->addWidget(new QLabel("PM"), 5, 6);
        }
        // row 7
        grid->addWidget(createLabel(p_osc1_start + off), 6, 0);
        grid->addWidget(createLabel(p_osc1_width + off), 6, 1);
        if (i > 0) {
            grid->addWidget(createLabel(p_osc1_pm + off), 6, 6);
        }
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setColumnStretch(7, 1);
        grid->setRowStretch(7, 1);
        return parent;
    }

    QWidget* createOscillators(QWidget* parent) {
        parent->setObjectName("oscillators");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        QTabWidget* tabs1 = new QTabWidget();
        tabs1->setTabPosition(QTabWidget::West);
        tabs1->addTab(createOscillator(new QGroupBox(), 0), "1");
        tabs1->addTab(createOscillator(new QGroupBox(), 2), "3");
        grid->addWidget(tabs1, 0, 0);
        // row 2
        QTabWidget* tabs2 = new QTabWidget();
        tabs2->setTabPosition(QTabWidget::West);
        tabs2->addTab(createOscillator(new QGroupBox(), 1), "2");
        tabs2->addTab(createOscillator(new QGroupBox(), 3), "4");
        grid->addWidget(tabs2, 1, 0);
        return parent;
    }

    QWidget* createMixer(QWidget* parent, int i) {
        int off = i * OSC_OFF;
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(p_osc1_level_a + off), 0, 0);
        grid->addWidget(createDial(p_osc1_level_b + off), 0, 1);
        // row 2
        grid->addWidget(new QLabel("Vol A"), 1, 0);
        grid->addWidget(new QLabel("Vol B"), 1, 1);
        // row 3
        grid->addWidget(createLabel(p_osc1_level_a + off), 2, 0);
        grid->addWidget(createLabel(p_osc1_level_b + off), 2, 1);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setRowStretch(3, 1);
        return parent;
    }

    QWidget* createMixer(QWidget* parent) {
        parent->setObjectName("mixer");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createIndex("1"), 0, 0);
        grid->addWidget(createMixer(new QGroupBox(), 0), 0, 1);
        // row 2
        grid->addWidget(createIndex("2"), 1, 0);
        grid->addWidget(createMixer(new QGroupBox(), 1), 1, 1);
        // row 3
        grid->addWidget(createIndex("3"), 2, 0);
        grid->addWidget(createMixer(new QGroupBox(), 2), 2, 1);
        // row 4
        grid->addWidget(createIndex("4"), 3, 0);
        grid->addWidget(createMixer(new QGroupBox(), 3), 3, 1);
        grid->setHorizontalSpacing(2);
        return parent;
    }

    QWidget* createOutput(QWidget* parent, int vol, int pan) {
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createDial(vol), 0, 0);
        grid->addWidget(createDial(pan), 0, 1);
        // row 2
        grid->addWidget(new QLabel("Vol"), 1, 0);
        grid->addWidget(new QLabel("Pan"), 1, 1);
        // row 3
        grid->addWidget(createLabel(vol), 2, 0);
        grid->addWidget(createLabel(pan), 2, 1);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setRowStretch(3, 1);
        return parent;
    }

    QWidget* createOutput(QWidget* parent) {
        parent->setObjectName("mixer");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createIndex("F1"), 0, 0);
        grid->addWidget(createOutput(new QGroupBox(), p_filter1_level, p_filter1_pan), 0, 1);
        // row 2
        grid->addWidget(createIndex("F2"), 1, 0);
        grid->addWidget(createOutput(new QGroupBox(), p_filter2_level, p_filter2_pan), 1, 1);
        // row 3
        grid->addWidget(createIndex("A"), 2, 0);
        grid->addWidget(createOutput(new QGroupBox(), p_bus_a_level, p_bus_a_pan), 2, 1);
        // row 4
        grid->addWidget(createIndex("B"), 3, 0);
        grid->addWidget(createOutput(new QGroupBox(), p_bus_b_level, p_bus_b_pan), 3, 1);
        grid->setHorizontalSpacing(2);
        return parent;
    }

    QDial* connectToFilter(QDial* dial, int i) {
        filterMapper.setMapping(dial, i);
        connect(dial, SIGNAL(valueChanged(int)), &filterMapper, SLOT(map()));
        return dial;
    }

    QPushButton* connectToFilter(QPushButton* button, int i) {
        filterMapper.setMapping(button, i);
        connect(button, SIGNAL(toggled(bool)), &filterMapper, SLOT(map()));
        return button;
    }

    QWidget* createFilter(QGroupBox* parent, int i) {
        int off = i * DCF_OFF;
        parent->setCheckable(true);
        connectBox(p_filter1_on + off, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        QComboBox* typeBox = createSelect(p_filter1_type + off, filter_types, 12);
        filterMapper.setMapping(typeBox, i);
        connect(typeBox, SIGNAL(currentIndexChanged(int)), &filterMapper, SLOT(map()));
        grid->addWidget(typeBox, 0, 0, 1, 2);
        grid->addWidget(createSelect(p_filter1_source + off, filter_sources, 2 + i), 0, 2, 1, 2);
        // row 2
        grid->addWidget(filter_wd[i] = new WaveDisplay(DCF_WIDTH, WAVE_HEIGHT), 1, 0, 3, 3);
        grid->addWidget(createDial(p_filter1_distortion + off), 1, 3);
        grid->addWidget(createDial(p_filter1_key_to_f + off), 1, 4);
        grid->addWidget(createDial(p_filter1_vel_to_f + off), 1, 5);
        // row 3
        grid->addWidget(new QLabel("Dist"), 2, 3);
        grid->addWidget(new QLabel("K > F"), 2, 4);
        grid->addWidget(new QLabel("V > F"), 2, 5);
        // row 4
        grid->addWidget(createLabel(p_filter1_distortion + off), 3, 3);
        grid->addWidget(createLabel(p_filter1_key_to_f + off), 3, 4);
        grid->addWidget(createLabel(p_filter1_vel_to_f + off), 3, 5);

        // row 5
        grid->addWidget(connectToFilter(createDial(p_filter1_freq + off), i), 4, 0);
        grid->addWidget(connectToFilter(createDial(p_filter1_q + off), i), 4, 1);
        //grid->addWidget(createDial(p_filter1_level + off), 4, 2);
        //grid->addWidget(createDial(p_filter1_pan + off), 4, 3);
        // row 6
        grid->addWidget(new QLabel("Freq"), 5, 0);
        grid->addWidget(new QLabel("Res"), 5, 1);
        //grid->addWidget(new QLabel("Vol"), 5, 2);
        //grid->addWidget(new QLabel("Pan"), 5, 3);
        // row 7
        grid->addWidget(createLabel(p_filter1_freq + off), 6, 0);
        grid->addWidget(createLabel(p_filter1_q + off), 6, 1);
        //grid->addWidget(createLabel(p_filter1_level + off), 6, 2);
        //grid->addWidget(createLabel(p_filter1_pan + off), 6, 3);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setRowStretch(7, 1);
        return parent;
    }

    QWidget* createFilters(QWidget* parent) {
        parent->setObjectName("filters");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createIndex("1"), 0, 0);
        grid->addWidget(createFilter(new QGroupBox(), 0), 0, 1);
        // row 2
        grid->addWidget(createIndex("2"), 1, 0);
        grid->addWidget(createFilter(new QGroupBox(), 1), 1, 1);
        grid->setColumnStretch(1, 1);
        grid->setHorizontalSpacing(2);
        return parent;
    }

    QDial* connectToEnv(QDial* dial, int i) {
        envMapper.setMapping(dial, i);
        connect(dial, SIGNAL(valueChanged(int)), &envMapper, SLOT(map()));
        return dial;
    }

    QWidget* createEnv(QGroupBox* parent, int i) {
        int off = i * ENV_OFF;
        parent->setCheckable(true);
        connectBox(p_env1_on + off, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(createToggle(p_env1_retrigger + off, "Retr"), 0, 0);
        // row 2
        grid->addWidget(env_wd[i] = new WaveDisplay(ENV_WIDTH, WAVE_HEIGHT), 1, 0, 3, 3);
        grid->addWidget(connectToEnv(createDial(p_env1_pre_delay + off), i), 1, 3);
        grid->addWidget(connectToEnv(createDial(p_env1_curve + off), i), 1, 4);
        // row 3
        grid->addWidget(new QLabel("Pre"), 2, 3);
        grid->addWidget(new QLabel("Curve"), 2, 4);
        // row 4
        grid->addWidget(createLabel(p_env1_pre_delay + off), 3, 3);
        grid->addWidget(createLabel(p_env1_curve + off), 3, 4);

        // row 5
        grid->addWidget(connectToEnv(createDial(p_env1_attack + off), i), 4, 0);
        grid->addWidget(connectToEnv(createDial(p_env1_hold + off), i), 4, 1);
        grid->addWidget(connectToEnv(createDial(p_env1_decay + off), i), 4, 2);
        grid->addWidget(connectToEnv(createDial(p_env1_sustain + off), i), 4, 3);
        grid->addWidget(connectToEnv(createDial(p_env1_release + off), i), 4, 4);
        // row 6
        grid->addWidget(new QLabel("A"), 5, 0);
        grid->addWidget(new QLabel("H"), 5, 1);
        grid->addWidget(new QLabel("D"), 5, 2);
        grid->addWidget(new QLabel("S"), 5, 3);
        grid->addWidget(new QLabel("R"), 5, 4);
        // row 7
        grid->addWidget(createLabel(p_env1_attack + off), 6, 0);
        grid->addWidget(createLabel(p_env1_hold + off), 6, 1);
        grid->addWidget(createLabel(p_env1_decay + off), 6, 2);
        grid->addWidget(createLabel(p_env1_sustain + off), 6, 3);
        grid->addWidget(createLabel(p_env1_release + off), 6, 4);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setColumnStretch(7, 1);
        grid->setRowStretch(7, 1);
        return parent;
    }

    QWidget* createEnvs(QWidget* parent) {
        parent->setObjectName("envs1");
        QGridLayout* grid = new QGridLayout(parent);
        grid->addWidget(createEnv(new QGroupBox(), 0), 0, 0);
        grid->addWidget(createEnv(new QGroupBox(), 1), 0, 1);
        grid->addWidget(createEnv(new QGroupBox(), 2), 0, 2);
        grid->addWidget(createEnv(new QGroupBox(), 3), 0, 3);
        grid->setRowStretch(1, 1);
        grid->setHorizontalSpacing(5);
        return parent;
    }

    QWidget* createMod(QWidget* parent, int off) {
        QGridLayout* grid = new QGridLayout(parent);
        for (int j = 0; j < 10; j += 2) {
            grid->addWidget(createSelect(p_mod1_src + off, mod_src_labels, M_SIZE), j, 0);
            grid->addWidget(createSelect(p_mod1_target + off, mod_target_labels, M_TARGET_SIZE), j + 1, 0);
            grid->addWidget(createDial(p_mod1_amount + off), j, 1, 2, 1);
            off += 3;
        }
        grid->setSpacing(1);
        grid->setRowStretch(10, 1);
        return parent;
    }

    QWidget* createMod(QWidget* parent) {
        parent->setObjectName("mod");
        QGridLayout* grid = new QGridLayout(parent);
        grid->addWidget(createMod(new QGroupBox(), 0), 0, 0);
        grid->addWidget(createMod(new QGroupBox(), 15), 0, 1);
        grid->addWidget(createMod(new QGroupBox(), 30), 0, 2);
        grid->addWidget(createMod(new QGroupBox(), 45), 0, 3);
        return parent;
    }

    QDial* connectToLfo(QDial* dial, int i) {
        lfoMapper.setMapping(dial, i);
        connect(dial, SIGNAL(valueChanged(int)), &lfoMapper, SLOT(map()));
        return dial;
    }

    QPushButton* connectToLfo(QPushButton* button, int i) {
        lfoMapper.setMapping(button, i);
        connect(button, SIGNAL(toggled(bool)), &lfoMapper, SLOT(map()));
        return button;
    }

    QWidget* createLfo(QGroupBox* parent, int i) {
        int off = i * LFO_OFF;
        parent->setCheckable(true);
        connectBox(p_lfo1_on + off, parent);
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        QComboBox* typeBox = createSelect(p_lfo1_type + off, lfo_types, 6);
        lfoMapper.setMapping(typeBox, i);
        connect(typeBox, SIGNAL(currentIndexChanged(int)), &lfoMapper, SLOT(map()));
        grid->addWidget(typeBox, 0, 0, 1, 2);
        grid->addWidget(createSelect(p_lfo1_reset_type + off, lfo_reset_types, 3), 0, 2, 1, 2);
        grid->addWidget(connectToLfo(createToggle(p_lfo1_inv + off, "Inv"), i), 0, 4);
        // row 2-4
        grid->addWidget(lfo_wd[i] = new WaveDisplay(LFO_WIDTH, WAVE_HEIGHT), 1, 0, 3, 3);
        // row 5
        grid->addWidget(createDial(p_lfo1_freq + off), 4, 0);
        grid->addWidget(connectToLfo(createDial(p_lfo1_start + off), i), 4, 1);
        grid->addWidget(connectToLfo(createDial(p_lfo1_width + off), i), 4, 2);
        grid->addWidget(createDial(p_lfo1_humanize + off), 4, 3);
        // row 6
        grid->addWidget(new QLabel("Freq"), 5, 0);
        grid->addWidget(new QLabel("Start"), 5, 1);
        grid->addWidget(new QLabel("Width"), 5, 2);
        grid->addWidget(new QLabel("Rand"), 5, 3);
        // row 7
        grid->addWidget(createLabel(p_lfo1_freq + off), 6, 0);
        grid->addWidget(createLabel(p_lfo1_start + off), 6, 1);
        grid->addWidget(createLabel(p_lfo1_width + off), 6, 2);
        grid->addWidget(createLabel(p_lfo1_humanize + off), 6, 3);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setColumnStretch(5, 1);
        grid->setRowStretch(7, 1);
        return parent;
    }

    QWidget* createLfos(QWidget* parent) {
        parent->setObjectName("lfos");
        QGridLayout* grid = new QGridLayout(parent);
        grid->addWidget(createLfo(new QGroupBox(), 0), 0, 0);
        grid->addWidget(createLfo(new QGroupBox(), 1), 0, 1);
        grid->addWidget(createLfo(new QGroupBox(), 2), 0, 2);
        grid->addWidget(createLfo(new QGroupBox(), 3), 0, 3);
        grid->setRowStretch(1, 1);
        grid->setHorizontalSpacing(5);
        return parent;
    }

    // UI to host
    Q_SLOT void portChange(int p) {
        float val = widgets[p]->get_value();
#ifndef ROGUI_UI_TEST
        write_control(p, val);
#endif
        QLabel* label = labels[p];
        if (label) {
            label->setNum(val);
        }
    }

    Q_SLOT void updateOsc(int i) {
        int type = (int)widgets[p_osc1_type + i * OSC_OFF]->get_value();
        float s = widgets[p_osc1_start + i * OSC_OFF]->get_value();
        float w = widgets[p_osc1_width + i * OSC_OFF]->get_value();
        bool inv = widgets[p_osc1_inv + i * OSC_OFF]->get_value() > 0.0f;

        osc.setStart(s);
        osc.resetPhase();
        float* buffer = osc_wd[i]->getSamples();
        int width = OSC_WIDTH;
        osc.process(type, 1.0f, w, w, buffer, width);
        if (inv) {
            for (int j = 0; j < width; j++) buffer[j] *= -1.0;
        }
        osc_wd[i]->repaint();
    }

    Q_SLOT void updateFilter(int i) {
        int type = (int)widgets[p_filter1_type + i * DCF_OFF]->get_value();
        float f = widgets[p_filter1_freq + i * DCF_OFF]->get_value();
        float q = widgets[p_filter1_q + i * DCF_OFF]->get_value();

        int width = 2 * DCF_WIDTH;
        float* in = fftIn[i];
        float* out = fftOut[i];
        for (int j = 0; j < width; j++) in[j] = 0;
        in[width / 2] = 1.0;

        // filter
        if (type < 8) {
            filter.moog.clear();
            filter.moog.setType(type);
            filter.moog.setCoefficients(f, q);
            filter.moog.process(in, in, width);
        } else {
            filter.svf.clear();
            filter.svf.setType(type - 8);
            filter.svf.setCoefficients(f, q);
            filter.svf.process(in, in, width);
        }

        fftwf_execute(fftPlan[i]);

        // post process fft results
        float* samples = filter_wd[i]->getSamples();
        float max_val = 0.0;
        for (int j = 0; j < DCF_WIDTH; j++) {
            samples[j] = sqrt(pow(out[j], 2) + pow(out[width - j], 2));
            max_val = std::max(max_val, samples[j]);
        }
        max_val /= 2.0;
        // normalize
        for (int j = 0; j < (width/2); j++) {
            samples[j] = samples[j] / max_val - 1.0;
        }
        filter_wd[i]->repaint();
    }

    Q_SLOT void updateEnv(int i) {
        float pre = widgets[p_env1_pre_delay + i * ENV_OFF]->get_value();
        float a = widgets[p_env1_attack + i * ENV_OFF]->get_value();
        float h = widgets[p_env1_hold + i * ENV_OFF]->get_value();
        float d = widgets[p_env1_decay + i * ENV_OFF]->get_value();
        float s = widgets[p_env1_sustain + i * ENV_OFF]->get_value();
        float r = widgets[p_env1_release + i * ENV_OFF]->get_value();

        float curve = widgets[p_env1_curve + i * ENV_OFF]->get_value();

        int width = ENV_WIDTH;
        float scale = ((float)width) / (pre + a + h + d + r);
        env.setPredelay(scale * pre);
        env.setAHDSR(scale * a, scale * h, scale * d, s, scale * r);
        env.setCurve(curve);
        env.on();
        float* buffer = env_wd[i]->getSamples();
        for (int j = 0; j < width; j++) {
            if (env.state() == 4) {
                env.off();
            }
            buffer[j] = env.tick() * 2.0 - 1.0;
        }
        env.off();
        env_wd[i]->repaint();
    }

    Q_SLOT void updateLfo(int i) {
        int type = (int)widgets[p_lfo1_type + i * LFO_OFF]->get_value();
        float s = widgets[p_lfo1_start + i * LFO_OFF]->get_value();
        float w = widgets[p_lfo1_width + i * LFO_OFF]->get_value();
        bool inv = widgets[p_lfo1_inv + i * LFO_OFF]->get_value() > 0.0f;

        lfo.setType(type);
        lfo.setStart(s);
        lfo.setWidth(w);
        lfo.reset();
        float* buffer = lfo_wd[i]->getSamples();
        int width = LFO_WIDTH;
        float scale = inv ? -1.0 : 1.0;
        for (int j = 0; j < width; j++) {
            buffer[j] = scale * lfo.tick();
        }
        lfo_wd[i]->repaint();
    }

  public:

    // host to UI
    void port_event(uint32_t port, uint32_t buffer_size, uint32_t format, const void* buffer) {
        if (port > 2) {
            widgets[port]->set_value(*static_cast<const float*>(buffer));
        }
    }

    rogueGUI(const char* URI) {
        for (int i = 0; i < p_n_ports; i++) {
            labels[i] = 0;
        }

        container().setObjectName("root");
        QWidget* oscs = createOscillators(new QGroupBox());
        oscs->setProperty("top", QVariant(true));
        QWidget* mixer = createMixer(new QGroupBox());
        mixer->setProperty("top", QVariant(true));
        QWidget* filters = createFilters(new QGroupBox());
        filters->setProperty("top", QVariant(true));
        QWidget* output = createOutput(new QGroupBox());
        output->setProperty("top", QVariant(true));

        QTabWidget* tabs = new QTabWidget();
        tabs->setTabPosition(QTabWidget::South);
        tabs->setObjectName("mainTab");
        tabs->setProperty("top", QVariant(true));
        tabs->addTab(createMain(new QFrame()), "Common");
        tabs->addTab(createEnvs(new QFrame()), "Envelopes");
        tabs->addTab(createLfos(new QFrame()), "LFOs");
        tabs->addTab(createMod(new QFrame()), "Matrix");
        tabs->addTab(createEffects(new QFrame()), "Effects");

        QFrame* logo = new QFrame();
        logo->setObjectName("logo");
        logo->setFixedSize(120, 39);

        // main grid
        QGridLayout* grid = new QGridLayout(&container());
        // row 1
        grid->addWidget(new QLabel("Oscillators"), 0, 0);
        grid->addWidget(new QLabel("Mixer"), 0, 1);
        grid->addWidget(new QLabel("Filters"), 0, 2);
        grid->addWidget(new QLabel("Output"), 0, 3);
        // row 2 & 3
        grid->addWidget(oscs, 1, 0, 2, 1);
        grid->addWidget(mixer, 1, 1, 2, 1);
        grid->addWidget(filters, 1, 2, 2, 1);
        grid->addWidget(output, 1, 3, 2, 1);
        // row 4
        grid->addWidget(tabs, 3, 0, 1, 4);
        // row 5
        //grid->addWidget(logo, 4, 3);
        grid->setRowStretch(4, 1);

        // connect
        connect(&mapper, SIGNAL(mapped(int)), SLOT(portChange(int)));
        connect(&oscMapper, SIGNAL(mapped(int)), SLOT(updateOsc(int)));
        connect(&filterMapper, SIGNAL(mapped(int)), SLOT(updateFilter(int)));
        connect(&envMapper, SIGNAL(mapped(int)), SLOT(updateEnv(int)));
        connect(&lfoMapper, SIGNAL(mapped(int)), SLOT(updateLfo(int)));

        // styles
        QFile file("styles/stylesheet.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        container().setStyleSheet(styleSheet);

        osc.setSamplerate(OSC_WIDTH);
        filter.setSamplerate(44100.0);
        lfo.setFreq(1.0);
        lfo.setSamplerate(LFO_WIDTH);

        // fft
        const int fft_width = 2 * DCF_WIDTH;
        for (int i = 0; i < 2; i++) {
            fftIn[i] = (float*) fftwf_malloc(sizeof(float) * fft_width);
            fftOut[i] = (float*) fftwf_malloc(sizeof(float) * fft_width);
            fftPlan[i] = fftwf_plan_r2r_1d(fft_width, fftIn[i], fftOut[i], FFTW_R2HC, FFTW_MEASURE);
        }

        // update displays
        for (int i = 0; i < 4; i++) updateOsc(i);
        for (int i = 0; i < 2; i++) updateFilter(i);
        for (int i = 0; i < 4; i++) updateEnv(i);
        for (int i = 0; i < 4; i++) updateLfo(i);

        // create dummy widgets for unused controls of osc1
        createDial(p_osc1_input);
        createDial(p_osc1_pm);
        createDial(p_osc1_sync);
        createDial(p_osc1_out_mod);

        for (int i = 3; i < p_n_ports; i++) {
            if (!widgets[i])
                std::cout << "Port "<< i << " not mapped!" << std::endl;
        }
    }

    ~rogueGUI() {
        // TODO delete widgets that are not managed by Qt
        for (int i = 0; i < 2; i++) {
            fftwf_free(fftIn[i]);
            fftwf_free(fftOut[i]);
            fftwf_destroy_plan(fftPlan[i]);
        }

        delete widgets[p_osc1_input];
        delete widgets[p_osc1_pm];
        delete widgets[p_osc1_sync];
        delete widgets[p_osc1_out_mod];
    }
};

#include "gui/rogue-gui.mcpp"

static int _ = rogueGUI::register_class("http://www.github.com/timowest/rogue/ui");


