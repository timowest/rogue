#ifndef QTWIDGETS_HPP
#define QTWIDGETS_HPP

#include <QDial>
#include <QPainter>
#include <QFrame>

class Widget {
  public:
    virtual float get_value() = 0;
    virtual void set_value(float val) = 0;
};

// CustomDial

// TODO get pen and brush color from stylesheet
// TODO use different colors if passivated
class CustomDial : public QDial, public Widget {
    static const int FULL = 5760;
    static const int HALF = 2880;

    static const QPainter::RenderHint paintFlags = QPainter::RenderHint(QPainter::Antialiasing
            | QPainter::SmoothPixmapTransform
            || QPainter::HighQualityAntialiasing);

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
        painter.setPen(QPen(QBrush("#666"), 3));
        painter.drawArc(2, 2, width - 4, height - 4, (1.125 - pos0 * 1.25) * HALF, -(pos - pos0) * 1.25 * HALF);
        //painter.drawArc(2, 2, width - 4, height - 4, 1.125 * HALF, -pos * 1.25 * HALF);

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
        pos0 = -min / (max - min);
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

// WaveDisplay

class WaveDisplay : public QFrame {
  public:
    WaveDisplay(int w, int h) {
        setFixedSize(w, h);
    }

};

#endif
