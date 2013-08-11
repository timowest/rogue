void oscillator_test() {
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
}
