#ifndef QTWIDGETS_HPP
#define QTWIDGETS_HPP

#include <QDial>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QPainter>

class Widget {
  public:
    virtual float get_value() = 0;
    virtual void set_value(float val) = 0;
};

// CustomDial

// TODO get pen and brush color from stylesheet
// TODO use different colors if passivated
class CustomDial : public QDial, public Widget {

    static const QPainter::RenderHint paintFlags = QPainter::RenderHint(QPainter::Antialiasing
                | QPainter::SmoothPixmapTransform
                || QPainter::HighQualityAntialiasing);

    static const int FULL = 5760;
    static const int HALF = 2880;
    float min, max, pos0, step;

  protected:
    void paintEvent(QPaintEvent *pe) {
        int width = this->width();
        int height = this->height();
        float pos = (value() - min) / (max - min);
        QPainter painter(this);
        painter.setRenderHints(paintFlags);

        // circle
        painter.setPen(QPen(QBrush("#bbb"), 1));
        painter.setBrush(QBrush(QColor("#bbb")));
        painter.drawEllipse(6, 6, width - 12, height - 12);

        // arc
        painter.setPen(QPen(isEnabled() ? QBrush("#666") : QBrush("#bbb"), 3));
        painter.drawArc(2, 2, width - 4, height - 4, (1.125 - pos0 * 1.25) * HALF, -(pos - pos0) * 1.25 * HALF);

        // line
        int radius = 0.5 * width - 6;
        double angle = (0.75 + pos * 1.5) * M_PI;
        painter.setPen(QPen(QBrush("#fff"), 2));
        painter.drawLine(
            width/2.0 + 0.2 * radius * cos(angle), height/2.0 + 0.2 * radius * sin(angle),
            width/2.0 + 0.8 * radius * cos(angle), height/2.0 + 0.8 * radius * sin(angle));
    }

  public:
    CustomDial(float _min, float _max, float _step, float _value) : QDial() {
        min = _min / _step;
        max = _max / _step;
        step = _step;
        pos0 = ((min > 0.0 ? min : 0.0) - min) / (max - min);
        setRange(min, max);
        setValue(_value / _step);
    }

    float get_value() {
        return value() * step;
    }

    void set_value(float v) {
        setValue(v / step);
    }

};

class CustomRadioButton : public QRadioButton, public Widget {

    float get_value() {
        return isChecked() ? 1.0f : 0.0f;
    }

    void set_value(float v) {
        setChecked(v > 0.0f);
    }
};

class CustomPushButton : public QPushButton, public Widget {

    float get_value() {
        return isChecked() ? 1.0f : 0.0f;
    }

    void set_value(float v) {
        setChecked(v > 0.0f);
    }
};

class CustomComboBox : public QComboBox, public Widget {

    float get_value() {
        return currentIndex();
    }

    void set_value(float v) {
        setCurrentIndex((int)v);
    }
};

class CustomSpinBox : public QDoubleSpinBox, public Widget {

    float get_value() {
        return value();
    }

    void set_value(float v) {
        setValue(v);
    }
};

class GroupBoxAdapter : public Widget {

    QGroupBox* box;

  public:
    GroupBoxAdapter(QGroupBox* _box) {
        box = box;
    }

    float get_value() {
        return box->isChecked() ? 1.0f : 0.0f;
    }

    void set_value(float v) {
        box->setChecked(v > 0.0f);
    }

};

// WaveDisplay

class WaveDisplay : public QFrame {
    static const QPainter::RenderHint paintFlags = QPainter::RenderHint(QPainter::Antialiasing
                | QPainter::SmoothPixmapTransform
                || QPainter::HighQualityAntialiasing);

    float* samples;

  protected:
    void paintEvent(QPaintEvent *pe) {
        if (!isEnabled()) {
            return;
        }
        int width = this->width();
        int height = this->height();
        QPainter painter(this);
        painter.setRenderHints(paintFlags);
        painter.setPen(QPen(QBrush("#000"), 1));
        for (int i = 1; i < width; i++) {
            painter.drawLine(i-1, (-0.45 * samples[i-1] + 0.5) * height,
                             i,   (-0.45 * samples[i] + 0.5) * height);

        }
    }

  public:
    WaveDisplay(int w, int h) {
        setFixedSize(w, h);
        samples = new float[w];
        for (int i = 0; i < w; i++) samples[i] = 0;
    }

    ~WaveDisplay() {
        delete samples;
    }

    float* getSamples() {
        return samples;
    }

};

#endif
