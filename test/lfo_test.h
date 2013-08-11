#include "wavutils.h"

void lfo_test() {
    char filename[50];
    float buffer[SIZE];
    float buffer2[SIZE];

    dsp::LFO lfo;
    lfo.setSamplerate(SR);
    lfo.setFreq(10.0);

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
}
