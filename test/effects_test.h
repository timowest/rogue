#include "wavutils.h"

void effects_test() {
    float buffer[SIZE];
    float buffer2[SIZE];
    buffer[0] = 1;
    buffer2[0] = 1;

    dsp::ChorusEffect chorus_fx;
    chorus_fx.setSamplerate(SR);

    dsp::PhaserEffect phaser_fx;
    phaser_fx.setSamplerate(SR);

    dsp::DelayEffect delay_fx;
    delay_fx.setSamplerate(SR);

    dsp::ReverbEffect reverb_fx;
    reverb_fx.setSamplerate(SR);

    // chorus
    chorus_fx.setCoefficients(0.01, 0.75, 0.3, 0.5, 0.5);
    chorus_fx.process(buffer, buffer2, SIZE);

    // phaser
    phaser_fx.setCoefficients(300.0, 0.5, 0.5, 0.5, 0.5);
    phaser_fx.process(buffer, buffer2, SIZE);

    // delay
    delay_fx.setCoefficients(120.0, 1.0, 1.0, 0.5, 0.5, 0.5, 10, 10000);
    delay_fx.process(buffer, buffer2, SIZE);

    // reverb
    reverb_fx.setCoefficients(0.9, 1.0, 5000.0, 0.1);
    reverb_fx.process(buffer, buffer2, SIZE);
}
