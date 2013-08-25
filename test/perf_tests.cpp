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
}
