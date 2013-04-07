#include <sndfile.hh>
#include "oscillator.cpp"

#define SR 44100
#define SIZE 44100
#define CHANNELS 1

int main() {
    const int FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    float buffer[SIZE];
    dsp::PhaseShaping osc;
    osc.setSamplerate(SR);
    osc.setFreq(440.0f);
    osc.setParams(0.5, 0.5);

    for (int i = 0; i < 10; i++) {
        osc.setType(i);
        osc.process(buffer, SR);

        // write wav
        char filename[30];
        sprintf(filename, "osc_type%i.wav", i);
        SndfileHandle outfile(filename, SFM_WRITE, FORMAT, CHANNELS, SR);
        if (!outfile) {
            return -1;
        }
        outfile.write(&buffer[0], SIZE);
    }
    return 0;
}
