#include <sndfile.hh>
#include "oscillator.cpp"
#include "filter.cpp"
#include "lfo.cpp"
#include "envelope.cpp"
#include "tables.cpp"

#include <iostream>

#define SR 44100.0
#define SIZE 44100
#define DOUBLE_SIZE 88200
#define CHANNELS 1

void write_wav(char* filename, float* buffer) {
    static const int FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SndfileHandle outfile(filename, SFM_WRITE, FORMAT, CHANNELS, SR);
    if (outfile) {
        outfile.write(&buffer[0], SIZE);
    }
}

void write_wav(char* filename, float* left, float* right) {
    static const int FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    float buffer[DOUBLE_SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer[2 * i] = left[i];
        buffer[2 * i + 1] = right[i];
    }
    SndfileHandle outfile(filename, SFM_WRITE, FORMAT, 2, SR);
    if (outfile) {
        outfile.write(&buffer[0], SIZE);
    }
}

int count_clicks(float* buffer) {
    float threshold = 0.4f;
    float diff = 0.0f;
    float diff2 = 0.0f;
    float diff3 = 0.0f;
    int clicks = 0;
    for (int i = 1; i < SIZE; i++) {
        diff = buffer[i] - buffer[i - 1];
        // one sample click
        if (fabs(diff) > threshold && fabs(diff2) > threshold &&
            ((diff < 0.0f && diff2 > 0.0f) || (diff > 0.0f && diff2 < 0.0))) {
            clicks++;
        // two sample click
        } else  if (fabs(diff) > threshold && fabs(diff2) < 0.1f && fabs(diff3) > threshold &&
            ((diff < 0.0f && diff3 > 0.0f) || (diff > 0.0f && diff3 < 0.0))) {
            clicks++;
        }
        diff3 = diff2;
        diff2 = diff;
    }
    return clicks;
}

// TODO split into component specific test functions

int main() {
    char filename[50];
    float buffer[SIZE];
    float buffer2[SIZE];

    dsp::Virtual va;
    va.setSamplerate(SR);
    va.setFreq(440.0f);

    dsp::AS as;
    as.setSamplerate(SR);
    as.setFreq(440.0f);

    dsp::Noise no;
    no.setSamplerate(SR);
    no.setFreq(440.0f);

    dsp::MoogFilter moog;
    moog.setSamplerate(SR);
    moog.setCoefficients(1000.0, 0.5);

    dsp::DCBlocker dcBlocker;
    dcBlocker.setSamplerate(SR);

    dsp::StateVariableFilter svf;
    svf.setSamplerate(SR);
    svf.setCoefficients(1000.0, 0.5);

    dsp::StateVariableFilter2 svf2;
    svf2.setSamplerate(SR);
    svf2.setCoefficients(1000.0, 0.5);

    dsp::LFO lfo;
    lfo.setSamplerate(SR);
    lfo.setFreq(10.0);

    dsp::AHDSR env;

    // va
    for (int i = 0; i < 34; i++) {
        va.reset();
        va.setType(i);
        va.process(buffer, SIZE);

        sprintf(filename, "wavs/va_%i.wav", i);
        write_wav(filename, buffer);
    }

    // as
    for (int i = 0; i < 3; i++) {
        as.reset();
        as.setType(i);
        as.process(buffer, SIZE);

        sprintf(filename, "wavs/as_%i.wav", i);
        write_wav(filename, buffer);
    }

    // noise
    for (int i = 0; i < 4; i++) {
        no.reset();
        no.setType(i);
        no.process(buffer, SIZE);

        sprintf(filename, "wavs/no_%i.wav", i);
        write_wav(filename, buffer);
    }

    // noise input
    float noise[SIZE];
    no.reset();
    no.setFreq(1000.0);
    no.setType(0);
    no.process(noise, SIZE);

    // moog
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 10; j++) {
            moog.clear();
            moog.setType(i);
            moog.setCoefficients(1000.0, float(j) * 0.1);
            moog.process(noise, buffer, SIZE);

            sprintf(filename, "wavs/moog_%i%i.wav", i, j);
            write_wav(filename, buffer);

            // TODO verify that output is not zero
        }
    }

    // svf
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            svf.clear();
            svf.setType(i);
            svf.setCoefficients(1000.0, float(j) * 0.1);
            svf.process(noise, buffer, SIZE);

            sprintf(filename, "wavs/svf_%i%i.wav", i, j);
            write_wav(filename, buffer);

            // TODO verify that output is not zero
        }
    }

    // svf2
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 10; j++) {
            svf2.clear();
            svf2.setType(i);
            svf2.setCoefficients(1000.0, float(j) * 0.1);
            svf2.process(noise, buffer, SIZE);

            sprintf(filename, "wavs/svf2_%i%i.wav", i, j);
            write_wav(filename, buffer);

            // TODO verify that output is not zero
        }
    }

    // lfos
    for (int i = 0; i < 5; i++) {
        lfo.reset();
        lfo.setType(i);
        //lfo.process(buffer, SIZE);
        for (int j = 0; j < SIZE; j++) {
            buffer[j] = lfo.tick();
        }

        sprintf(filename, "wavs/lfo_%i.wav", i);
        write_wav(filename, buffer);
    }

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

    return 0;
}
