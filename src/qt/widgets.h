#ifndef QTWIDGETS_HPP
#define QTWIDGETS_HPP

#include <QDial>
#include <QPainter>
#include <QFrame>

// CustomDial

// TODO get pen and brush color from stylesheet
// TODO use different colors if passivated
class CustomDial : public QDial {
    static const int FULL = 5760;
    static const int HALF = 2880;

    static const QPainter::RenderHint paintFlags = QPainter::RenderHint(QPainter::Antialiasing
            | QPainter::SmoothPixmapTransform
            || QPainter::HighQualityAntialiasing);

    float min, max, pos0;

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
            width/2 + 0.2 * radius * cos(angle), height/2 + 0.2 * radius * sin(angle),
            width/2 + 0.8 * radius * cos(angle), height/2 + 0.8 * radius * sin(angle));
    }

  public:
    CustomDial(int _min, int _max, int _value) : QDial() {
        setMinimum(_min);
        setMaximum(_max);
        setValue(_value);

        min = _min;
        max = _max;
        pos0 = -min / (max - min);
    }

};

// WaveDisplay

// 120/60, 100/60
class WaveDisplay : public QFrame {
  public:
    WaveDisplay(int w, int h) {
        setFixedSize(w, h);
    }

};

#endif
