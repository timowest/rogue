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
    static const int DCF_WIDTH = 100;
    static const int ENV_WIDTH = 100;
    static const int LFO_WIDTH = 100;

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
    WaveDisplay* env_wd[5];
    WaveDisplay* lfo_wd[3];

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
        // row 3
        grid->addWidget(createLabel(p_volume), 2, 0);
        grid->addWidget(createLabel(p_bus_a_level), 2, 1);
        grid->addWidget(createLabel(p_bus_a_pan), 2, 2);
        grid->addWidget(createLabel(p_bus_b_level), 2, 3);
        grid->addWidget(createLabel(p_bus_b_pan), 2, 4);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setColumnStretch(5, 1);
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
        // row 2
        grid->addWidget(createReverb(new QGroupBox("Reverb")), 1, 0);
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
        grid->addWidget(createToggle(p_osc1_sync + off, "Sync"), 0, 4);
        grid->addWidget(createToggle(p_osc1_free + off, "Free"), 0, 5);
        // row 2
        grid->addWidget(createDial(p_osc1_coarse + off), 1, 0);
        grid->addWidget(createDial(p_osc1_fine + off), 1, 1);
        grid->addWidget(createDial(p_osc1_ratio + off), 1, 2);
        grid->addWidget(createDial(p_osc1_level + off), 1, 3);
        grid->addWidget(osc_wd[i] = new WaveDisplay(OSC_WIDTH, WAVE_HEIGHT), 1, 4, 3, 3);
        // row 3
        grid->addWidget(new QLabel("Coarse"), 2, 0);
        grid->addWidget(new QLabel("Fine"), 2, 1);
        grid->addWidget(new QLabel("Ratio"), 2, 2);
        grid->addWidget(new QLabel("Level"), 2, 3);
        // row 4
        grid->addWidget(createLabel(p_osc1_coarse + off), 3, 0);
        grid->addWidget(createLabel(p_osc1_fine + off), 3, 1);
        grid->addWidget(createLabel(p_osc1_ratio + off), 3, 2);
        grid->addWidget(createLabel(p_osc1_level + off), 3, 3);
        // row 5
        grid->addWidget(connectToOsc(createDial(p_osc1_start + off), i), 4, 0);
        grid->addWidget(connectToOsc(createDial(p_osc1_width + off), i), 4, 1);
        grid->addWidget(createDial(p_osc1_level_a + off), 4, 2);
        grid->addWidget(createDial(p_osc1_level_b + off), 4, 3);
        if (i > 0) {
            grid->addWidget(createSelect(p_osc1_input + off, nums, i), 4, 4);
            grid->addWidget(createSelect(p_osc1_out_mod + off, out_mod, 4), 4, 5);
            grid->addWidget(createDial(p_osc1_pm + off), 4, 6);
        }
        // row 6
        grid->addWidget(new QLabel("Start"), 5, 0);
        grid->addWidget(new QLabel("Width"), 5, 1);
        grid->addWidget(new QLabel("Vol A"), 5, 2);
        grid->addWidget(new QLabel("Vol B"), 5, 3);
        if (i > 0) {
            grid->addWidget(new QLabel("Input"), 5, 4);
            grid->addWidget(new QLabel("Mod"), 5, 5);
            grid->addWidget(new QLabel("PM"), 5, 6);
        }
        // row 7
        grid->addWidget(createLabel(p_osc1_start + off), 6, 0);
        grid->addWidget(createLabel(p_osc1_width + off), 6, 1);
        grid->addWidget(createLabel(p_osc1_level_a + off), 6, 2);
        grid->addWidget(createLabel(p_osc1_level_b + off), 6, 3);
        if (i > 0) {
            grid->addWidget(createLabel(p_osc1_pm + off), 6, 6);
        }
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setRowStretch(7, 1);
        return parent;
    }

    QWidget* createOscillators(QWidget* parent) {
        parent->setObjectName("oscillators");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(new QLabel("1"), 0, 0);
        grid->addWidget(createOscillator(new QGroupBox(), 0), 0, 1);
        grid->addWidget(createOscillator(new QGroupBox(), 2), 0, 2);
        grid->addWidget(new QLabel("3"), 0, 3);
        // row 2
        grid->addWidget(new QLabel("2"), 1, 0);
        grid->addWidget(createOscillator(new QGroupBox(), 1), 1, 1);
        grid->addWidget(createOscillator(new QGroupBox(), 3), 1, 2);
        grid->addWidget(new QLabel("4"), 1, 3);
        grid->setColumnStretch(1, 1);
        grid->setColumnStretch(2, 1);
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
        grid->addWidget(connectToFilter(createDial(p_filter1_freq + off), i), 1, 0);
        grid->addWidget(connectToFilter(createDial(p_filter1_q + off), i), 1, 1);
        grid->addWidget(createDial(p_filter1_level + off), 1, 2);
        grid->addWidget(createDial(p_filter1_pan + off), 1, 3);
        grid->addWidget(filter_wd[i] = new WaveDisplay(DCF_WIDTH, WAVE_HEIGHT), 1, 4, 3, 3);
        // row 3
        grid->addWidget(new QLabel("Freq"), 2, 0);
        grid->addWidget(new QLabel("Res"), 2, 1);
        grid->addWidget(new QLabel("Vol"), 2, 2);
        grid->addWidget(new QLabel("Pan"), 2, 3);
        // row 4
        grid->addWidget(createLabel(p_filter1_freq + off), 3, 0);
        grid->addWidget(createLabel(p_filter1_q + off), 3, 1);
        grid->addWidget(createLabel(p_filter1_level + off), 3, 2);
        grid->addWidget(createLabel(p_filter1_pan + off), 3, 3);
        // row 5
        grid->addWidget(createDial(p_filter1_distortion + off), 4, 0);
        grid->addWidget(createDial(p_filter1_key_to_f + off), 4, 1);
        grid->addWidget(createDial(p_filter1_vel_to_f + off), 4, 2);
        // row 6
        grid->addWidget(new QLabel("Dist"), 5, 0);
        grid->addWidget(new QLabel("K > F"), 5, 1);
        grid->addWidget(new QLabel("V > F"), 5, 2);
        // row 7
        grid->addWidget(createLabel(p_filter1_distortion + off), 6, 0);
        grid->addWidget(createLabel(p_filter1_key_to_f + off), 6, 1);
        grid->addWidget(createLabel(p_filter1_vel_to_f + off), 6, 2);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setRowStretch(7, 1);
        return parent;
    }

    QWidget* createFilters(QWidget* parent) {
        parent->setObjectName("filters");
        QGridLayout* grid = new QGridLayout(parent);
        // row 1
        grid->addWidget(new QLabel("1"), 0, 0);
        grid->addWidget(createFilter(new QGroupBox(), 0), 0, 1);
        // row 2
        grid->addWidget(new QLabel("2"), 1, 0);
        grid->addWidget(createFilter(new QGroupBox(), 1), 1, 1);
        grid->setColumnStretch(1, 1);
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
        grid->addWidget(connectToEnv(createDial(p_env1_attack + off), i), 1, 0);
        grid->addWidget(connectToEnv(createDial(p_env1_decay + off), i), 1, 1);
        grid->addWidget(connectToEnv(createDial(p_env1_sustain + off), i), 1, 2);
        grid->addWidget(connectToEnv(createDial(p_env1_release + off), i), 1, 3);
        grid->addWidget(env_wd[i] = new WaveDisplay(ENV_WIDTH, WAVE_HEIGHT), 1, 4, 3, 3);
        // row 3
        grid->addWidget(new QLabel("A"), 2, 0);
        grid->addWidget(new QLabel("D"), 2, 1);
        grid->addWidget(new QLabel("S"), 2, 2);
        grid->addWidget(new QLabel("R"), 2, 3);
        // row 4
        grid->addWidget(createLabel(p_env1_attack + off), 3, 0);
        grid->addWidget(createLabel(p_env1_decay + off), 3, 1);
        grid->addWidget(createLabel(p_env1_sustain + off), 3, 2);
        grid->addWidget(createLabel(p_env1_release + off), 3, 3);
        // row 5
        grid->addWidget(connectToEnv(createDial(p_env1_hold + off), i), 4, 0);
        grid->addWidget(connectToEnv(createDial(p_env1_pre_delay + off), i), 4, 1);
        grid->addWidget(connectToEnv(createDial(p_env1_curve + off), i), 4, 2);
        // row 6
        grid->addWidget(new QLabel("Hold"), 5, 0);
        grid->addWidget(new QLabel("Pre"), 5, 1);
        grid->addWidget(new QLabel("Curve"), 5, 2);
        // row 7
        grid->addWidget(createLabel(p_env1_hold + off), 6, 0);
        grid->addWidget(createLabel(p_env1_pre_delay + off), 6, 1);
        grid->addWidget(createLabel(p_env1_curve + off), 6, 2);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setRowStretch(7, 1);
        return parent;
    }

    QWidget* createEnvs(QWidget* parent) {
        parent->setObjectName("envs");
        // 1
        QFrame* frame1 = new QFrame();
        QGridLayout* grid1 = new QGridLayout(frame1);
        grid1->addWidget(new QLabel("1"), 0, 0);
        grid1->addWidget(createEnv(new QGroupBox(), 0), 0, 1);
        grid1->addWidget(new QLabel("2"), 0, 2);
        grid1->addWidget(createEnv(new QGroupBox(), 1), 0, 3);
        grid1->addWidget(new QLabel("3"), 0, 4);
        grid1->addWidget(createEnv(new QGroupBox(), 2), 0, 5);
        grid1->setColumnStretch(6, 1);
        // 2
        QFrame* frame2 = new QFrame();
        QGridLayout* grid2 = new QGridLayout(frame2);
        grid2->addWidget(new QLabel("4"), 0, 0);
        grid2->addWidget(createEnv(new QGroupBox(), 3), 0, 1);
        grid2->addWidget(new QLabel("5"), 0, 2);
        grid2->addWidget(createEnv(new QGroupBox(), 4), 0, 3);
        grid2->setColumnStretch(4, 1);

        QHBoxLayout* layout = new QHBoxLayout(parent);
        QTabWidget* tabs = new QTabWidget();
        tabs->setTabPosition(QTabWidget::West);
        tabs->addTab(frame1, "1-3");
        tabs->addTab(frame2, "4-5");
        layout->addWidget(tabs);
        return parent;
    }

    QWidget* createMod(QWidget* parent, int i) {
        int off = i * (p_mod8_src - p_mod1_src);
        QGridLayout* grid = new QGridLayout(parent);
        int max = i < 2 ? 7 : 6;
        for (int j = 0; j < max; j++) {
            grid->addWidget(createSelect(p_mod1_src + off, mod_src_labels, M_SIZE), j, 0);
            grid->addWidget(createSelect(p_mod1_target + off, mod_target_labels, M_TARGET_SIZE), j, 1);
            grid->addWidget(createSpin(p_mod1_amount + off), j, 2);
            off += 3;
        }
        grid->setSpacing(2);
        grid->setRowStretch(max, 1);
        return parent;
    }

    QWidget* createMod(QWidget* parent) {
        parent->setObjectName("mod");
        QHBoxLayout* layout = new QHBoxLayout(parent);
        layout->addWidget(createMod(new QGroupBox(), 0));
        layout->addWidget(createMod(new QGroupBox(), 1));
        layout->addWidget(createMod(new QGroupBox(), 2));
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
        // row 2
        grid->addWidget(createDial(p_lfo1_freq + off), 1, 0);
        grid->addWidget(connectToLfo(createDial(p_lfo1_start + off), i), 1, 1);
        grid->addWidget(connectToLfo(createDial(p_lfo1_width + off), i), 1, 2);
        grid->addWidget(createDial(p_lfo1_humanize + off), 1, 3);
        grid->addWidget(lfo_wd[i] = new WaveDisplay(LFO_WIDTH, WAVE_HEIGHT), 1, 4, 3, 3);
        // row 3
        grid->addWidget(new QLabel("Freq"), 2, 0);
        grid->addWidget(new QLabel("Start"), 2, 1);
        grid->addWidget(new QLabel("Width"), 2, 2);
        grid->addWidget(new QLabel("Rand"), 2, 3);
        // row 4
        grid->addWidget(createLabel(p_lfo1_freq + off), 3, 0);
        grid->addWidget(createLabel(p_lfo1_start + off), 3, 1);
        grid->addWidget(createLabel(p_lfo1_width + off), 3, 2);
        grid->addWidget(createLabel(p_lfo1_humanize + off), 3, 3);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(0);
        grid->setRowStretch(4, 1);
        return parent;
    }

    QWidget* createLfos(QWidget* parent) {
        parent->setObjectName("lfos");
        QGridLayout* grid = new QGridLayout(parent);
        grid->addWidget(new QLabel("1"), 0, 0);
        grid->addWidget(createLfo(new QGroupBox(), 0), 0, 1);
        grid->addWidget(new QLabel("2"), 0, 2);
        grid->addWidget(createLfo(new QGroupBox(), 1), 0, 3);
        grid->addWidget(new QLabel("3"), 0, 4);
        grid->addWidget(createLfo(new QGroupBox(), 2), 0, 5);
        grid->setColumnStretch(6, 1);
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
        float val = widgets[p]->get_value();
        //writeControl(p, widgets[p]->get_value());
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

    rogueGUI() {
        for (int i = 0; i < p_n_ports; i++) {
            labels[i] = 0;
        }

        container().setObjectName("root");
        QWidget* main = createMain(new QGroupBox("Main"));
        main->setProperty("top", QVariant(true));
        QWidget* oscs = createOscillators(new QGroupBox("Oscillators"));
        oscs->setProperty("top", QVariant(true));
        QWidget* filters = createFilters(new QGroupBox("Filters"));
        filters->setProperty("top", QVariant(true));

        QTabWidget* tabs = new QTabWidget();
        tabs->setObjectName("mainTab");
        tabs->setProperty("top", QVariant(true));
        tabs->addTab(createLfos(new QFrame()), "LFOs");
        tabs->addTab(createEnvs(new QFrame()), "Envelopes");
        tabs->addTab(createMod(new QFrame()), "Modulation");
        tabs->addTab(createEffects(new QFrame()), "Effects");

        // main grid
        QGridLayout* grid = new QGridLayout(&container());
        // row 1 & 2
        grid->addWidget(oscs, 0, 0, 2, 2);
        grid->addWidget(filters, 0, 2, 2, 1);
        // row 3
        grid->addWidget(tabs, 2, 0, 1, 3);
        // row 4
        grid->addWidget(main, 3, 0, 1, 3);
        grid->setRowStretch(0, 1);
        grid->setRowStretch(1, 1);

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
        for (int i = 0; i < 5; i++) updateEnv(i);
        for (int i = 0; i < 3; i++) updateLfo(i);
    }

    ~rogueGUI() {
        // TODO delete widgets that are not managed by Qt
        for (int i = 0; i < 2; i++) {
            fftwf_free(fftIn[i]);
            fftwf_free(fftOut[i]);
            fftwf_destroy_plan(fftPlan[i]);
        }
    }
};

#include "gui/rogue-gui.mcpp"
