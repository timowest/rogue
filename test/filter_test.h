#include "wavutils.h"

void filter_test() {
    char filename[50];
    float buffer[SIZE];
    float buffer2[SIZE];

    // for input
    dsp::Noise no;
    no.setSamplerate(SR);
    no.setFreq(440.0f);

    dsp::MoogFilter moog;
    moog.setSamplerate(SR);
    moog.setCoefficients(1000.0, 0.5);

    dsp::MoogFilter2 moog2;
    moog2.setSamplerate(SR);
    moog2.setCoefficients(1000.0, 0.5);

    dsp::StateVariableFilter svf;
    svf.setSamplerate(SR);
    svf.setCoefficients(1000.0, 0.5);

    dsp::StateVariableFilter2 svf2;
    svf2.setSamplerate(SR);
    svf2.setCoefficients(1000.0, 0.5);

    // noise input
    float noise[SIZE];
    no.setFreq(1000.0);
    no.setType(0);
    no.process(noise, buffer, SIZE);

    // moog
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 10; j++) {
            moog.clear();
            moog.setType(i);
            moog.setCoefficients(1000.0, float(j) * 0.1);
            moog.process(noise, buffer, SIZE);

            bool zero = true;
            for (int i = 0; i < SIZE; i++) {
                zero &= buffer[i] == 0.0f;
            }
            if (zero) {
                error("moog is silent %", i);
            }

            sprintf(filename, "wavs/filter/moog_%i%i.wav", i, j);
            write_wav(filename, buffer);
        }
    }

    // moog2
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 10; j++) {
            moog2.clear();
            //moog2.setType(i);
            moog2.setCoefficients(1000.0, float(j) * 0.1);
            moog2.process(noise, buffer, SIZE);

            bool zero = true;
            for (int i = 0; i < SIZE; i++) {
                zero &= buffer[i] == 0.0f;
            }
            if (zero) {
                error("moog2 is silent %", i);
            }

            sprintf(filename, "wavs/filter/moog2_%i%i.wav", i, j);
            write_wav(filename, buffer);
        }
    }

    // svf
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            svf.clear();
            svf.setType(i);
            svf.setCoefficients(1000.0, float(j) * 0.1);
            svf.process(noise, buffer, SIZE);

            bool zero = true;
            for (int i = 0; i < SIZE; i++) {
                zero &= buffer[i] == 0.0f;
            }
            if (zero) {
                error("svf is silent %i", i);
            }

            sprintf(filename, "wavs/filter/svf_%i%i.wav", i, j);
            write_wav(filename, buffer);
        }
    }

    // svf2
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            svf2.clear();
            svf2.setType(i);
            svf2.setCoefficients(1000.0, float(j) * 0.1);
            svf2.process(noise, buffer, SIZE);

            bool zero = true;
            for (int i = 0; i < SIZE; i++) {
                zero &= buffer[i] == 0.0f;
            }
            if (zero) {
                error("svf2 is silent %i", i);
            }

            sprintf(filename, "wavs/filter/svf2_%i%i.wav", i, j);
            write_wav(filename, buffer);
        }
    }
}
