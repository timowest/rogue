#include <sndfile.hh>
#include "oscillator.cpp"
#include "filter.cpp"

#define SR 44100
#define SIZE 44100
#define CHANNELS 1

void write_wav(char* filename, float* buffer) {
    static const int FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SndfileHandle outfile(filename, SFM_WRITE, FORMAT, CHANNELS, SR);
    if (outfile) {
        outfile.write(&buffer[0], SIZE);
    }
}

int main() {
    char filename[50];
    float buffer[SIZE];
    dsp::PhaseShaping osc;
    osc.setSamplerate(SR);
    osc.setFreq(440.0f);
    osc.setParams(0.5, 0.5);

    dsp::MoogFilter moog;
    moog.setSamplerate(SR);
    moog.setCoefficients(1000.0, 0.5);

    dsp::StateVariableFilter svf;
    svf.setSamplerate(SR);
    svf.setCoefficients(1000.0, 0.5);

    // oscs
    for (int i = 0; i < 10; i++) {
        osc.reset();
        osc.setType(i);
        osc.process(buffer, SIZE);

        sprintf(filename, "osc_%i.wav", i);
        write_wav(filename, buffer);
    }

    // noise input
    float noise[SIZE];
    osc.reset();
    osc.setFreq(1000.0);
    osc.setType(9);
    osc.process(noise, SIZE);

    // moog
    for (int i = 0; i < 8; i++) {
        moog.clear();
        moog.setType(i);
        moog.process(noise, buffer, SIZE);

        sprintf(filename, "moog_%i.wav", i);
        write_wav(filename, buffer);
    }

    // svf
    for (int i = 0; i < 4; i++) {
        svf.clear();
        svf.setType(i);
        svf.process(noise, buffer, SIZE);

        sprintf(filename, "svf_%i.wav", i);
        write_wav(filename, buffer);
    }

    // lfos
    // TODO

    // envs
    // TODO

    return 0;
}
