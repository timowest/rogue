#include <time.h>

#include "oscillator.cpp"
#include "filter.cpp"
#include "lfo.cpp"
#include "envelope.cpp"
#include "tables.cpp"

#define SR 44100.0
#define SIZE 44100

int main() {
    float buffer[SIZE];
    dsp::PhaseShaping osc;
    osc.setSamplerate(SR);
    osc.setFreq(440.0f);
    osc.setWidth(0.5f);
    osc.setParams(1.0f, 0.0f);

    dsp::MoogFilter moog;
    moog.setSamplerate(SR);
    moog.setCoefficients(1000.0, 0.5);

    dsp::DCBlocker dcBlocker;
    dcBlocker.setSamplerate(SR);

    dsp::StateVariableFilter svf;
    svf.setSamplerate(SR);
    svf.setCoefficients(1000.0, 0.5);

    dsp::LFO lfo;
    lfo.setSamplerate(SR);
    lfo.setFreq(10.0);

    dsp::AHDSR env;

    // oscs
    float params[] = {0.0f, 0.02f, 0.1f, 0.2f, 0.25f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.75f, 0.8f, 0.9f, 0.98f,
                      1.0f, 1.02f, 1.1f, 1.2f, 1.25f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.75f, 1.8f, 1.9f, 1.98f};
    for (int i = 0; i < 10; i++) { // type
        time_t start = time(NULL);
        for (int j = 0; j < 28; j++) { // param1
            for (int k = 0; k < 15; k++) { // param2 & width
                osc.reset();
                osc.setBandlimit(true);
                osc.setType(i);
                osc.setParams(params[j], params[k]);
                osc.setWidth(params[k]);
                osc.process(buffer, SIZE);
            }
        }

        printf("osc%i, time elapsed: %d\n", i, (time(NULL) - start));
    }

    // noise input
    float noise[SIZE];
    osc.reset();
    osc.setFreq(1000.0);
    osc.setType(9);
    osc.process(noise, SIZE);

    // moog
    for (int i = 0; i < 8; i++) {
        time_t start = time(NULL);
        moog.clear();
        moog.setType(i);
        moog.process(noise, buffer, SIZE);

        printf("moog%i, time elapsed: %d\n", i, (time(NULL) - start));
    }

    // svf
    for (int i = 0; i < 4; i++) {
        time_t start = time(NULL);
        svf.clear();
        svf.setType(i);
        svf.process(noise, buffer, SIZE);

        printf("svf%i, time elapsed: %d\n", i, (time(NULL) - start));
    }

    // lfos
    for (int i = 0; i < 5; i++) {
        time_t start = time(NULL);
        lfo.reset();
        lfo.setType(i);
        //lfo.process(buffer, SIZE);
        for (int j = 0; j < SIZE; j++) {
            buffer[j] = lfo.tick();
        }

        printf("lfo%i, time elapsed: %d\n", i, (time(NULL) - start));
    }

    // envs
    float curves[] = {0.0f, 0.1f, 0.25f, 0.4f, 0.5f, 0.6f, 0.75f, 0.9f, 1.0f};
    env.setAHDSR(0.1 * SR, 0.1 * SR, 0.3 * SR, 0.4, 0.5 * SR);
    for (int i = 0; i < 9; i++) {
        env.setCurve(curves[i]);

        time_t start = time(NULL);
        env.on();
        for (int j = 0; j < 0.5 * SIZE; j++) {
            buffer[j] = env.tick();
        }
        env.off();
        for (int j = 0.5* SIZE; j < SIZE; j++) {
            buffer[j] = env.tick();
        }

        printf("env%i, time elapsed: %d\n", i, (time(NULL) - start));
    }


    return 0;
}
