#include <iostream>
#include <omp.h>
#include "filter.cpp"
#include "oscillator.cpp"
#include "tables.cpp"

#define SIZE 64
#define SR 44100
#define ITERATIONS 1000000

void log(const char* label, int type, float duration) {
    std::cout << label << " " << type << " " << duration << std::endl;
}

int main() {
    float buffer[SIZE];
    float sync[SIZE];

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

    // va
    for (int i = 0; i < 34; i++) {
        va.reset();
        va.setType(i);
        double start = omp_get_wtime();
        for (int j = 0; j < ITERATIONS; j++) {
            va.process(buffer, sync, SIZE);
        }
        double end = omp_get_wtime();
        log("va", i, end - start);
    }

    // as
    for (int i = 0; i < 3; i++) {
        as.reset();
        as.setType(i);
        double start = omp_get_wtime();
        for (int j = 0; j < ITERATIONS; j++) {
            as.process(buffer, sync, SIZE);
        }
        double end = omp_get_wtime();
        log("as", i, end - start);
    }

    // noise
    for (int i = 0; i < 4; i++) {
        no.reset();
        no.setType(i);
        double start = omp_get_wtime();
        for (int j = 0; j < ITERATIONS; j++) {
            no.process(buffer, sync, SIZE);
        }
        double end = omp_get_wtime();
        log("no", i, end - start);
    }

    // moog
    for (int i = 0; i < 8; i++) {
        moog.clear();
        moog.setType(i);
        double start = omp_get_wtime();
        for (int j = 0; j < ITERATIONS; j++) {
            moog.process(noise, buffer, SIZE);
        }
        double end = omp_get_wtime();
        log("moog", i, end - start);
    }

    // svf
    for (int i = 0; i < 4; i++) {
        svf.clear();
        svf.setType(i);
        double start = omp_get_wtime();
        for (int j = 0; j < ITERATIONS; j++) {
            svf.process(noise, buffer, SIZE);
        }
        double end = omp_get_wtime();
        log("svf", i, end - start);
    }

    // svf2
    for (int i = 0; i < 4; i++) {
        svf2.clear();
        svf2.setType(i);
        double start = omp_get_wtime();
        for (int j = 0; j < ITERATIONS; j++) {
            svf2.process(noise, buffer, SIZE);
        }
        double end = omp_get_wtime();
        log("svf2", i, end - start);
    }
}
