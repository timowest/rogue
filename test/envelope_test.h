#include "wavutils.h"

void envelope_test() {
    char filename[50];
    float buffer[SIZE];
    float buffer2[SIZE];

    dsp::AHDSR env;

    // envs
    float curves[] = {0.0f, 0.1f, 0.25f, 0.4f, 0.5f, 0.6f, 0.75f, 0.9f, 1.0f};
    env.setAHDSR(0.1 * SR, 0.1 * SR, 0.3 * SR, 0.4, 0.5 * SR);
    for (int i = 0; i < 9; i++) {
        env.setCurve(curves[i]);
        env.on();
        for (int j = 0; j < 0.5 * SIZE; j++) {
            buffer[j] = env.tick();
        }
        env.off();
        for (int j = 0.5* SIZE; j < SIZE; j++) {
            buffer[j] = env.tick();
        }
        sprintf(filename, "wavs/env_%i.wav", i);
        write_wav(filename, buffer);
    }

    // with predelay
    env.setCurve(0.5f);
    env.setPredelay(2000.0f);
    env.on();
    for (int j = 0; j < 0.5 * SIZE; j++) {
        buffer[j] = env.tick();
    }
    env.off();
    for (int j = 0.5* SIZE; j < SIZE; j++) {
        buffer[j] = env.tick();
    }
    sprintf(filename, "wavs/env_pre_%i.wav", 0);
    write_wav(filename, buffer);

    // retrigger
    env.setCurve(0.5f);
    env.setPredelay(0.0f);
    env.on();
    for (int j = 0; j < 0.25 * SIZE; j++) {
        buffer[j] = env.tick();
    }
    env.off();
    for (int j = 0.25 * SIZE; j < 0.5 * SIZE; j++) {
        buffer[j] = env.tick();
    }
    env.on();
    for (int j = 0.5 * SIZE; j < 0.75 * SIZE; j++) {
        buffer[j] = env.tick();
    }
    env.off();
    for (int j = 0.75 * SIZE; j < SIZE; j++) {
        buffer[j] = env.tick();
    }
    sprintf(filename, "wavs/env_retrigger_%i.wav", 0);
    write_wav(filename, buffer);
}
