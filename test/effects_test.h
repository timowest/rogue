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
    write_wav((char*)"wavs/fx/chorus.wav", buffer);

    for (uint i = 0; i < SIZE; i++) {
        buffer[i] = buffer2[i] = (i == 0) ? 1 : 0;
    }

    // phaser
    phaser_fx.setCoefficients(300.0, 0.5, 0.5, 0.5, 0.5);
    phaser_fx.process(buffer, buffer2, SIZE);
    write_wav((char*)"wavs/fx/phaser.wav", buffer);

    for (uint i = 0; i < SIZE; i++) {
        buffer[i] = buffer2[i] = (i == 0) ? 1 : 0;
    }

    // delay
    delay_fx.setCoefficients(120.0, 1.0, 1.0, 0.5, 0.5, 0.5, 10, 10000);
    delay_fx.process(buffer, buffer2, SIZE);
    write_wav((char*)"wavs/fx/delay.wav", buffer);

    for (uint i = 0; i < SIZE; i++) {
        buffer[i] = buffer2[i] = (i == 0) ? 1 : 0;
    }

    // reverb
    reverb_fx.setCoefficients(0.01, 0.5, 100, 5000, 0.5);
    reverb_fx.process(buffer, buffer2, SIZE);
    write_wav((char*)"wavs/fx/reverb.wav", buffer);
}
