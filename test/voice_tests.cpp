#include <sndfile.hh>
#include "oscillator.cpp"
#include "filter.cpp"
#include "lfo.cpp"
#include "envelope.cpp"
#include "voice.cpp"
#include "tables.cpp"

#define SR 44100.0
#define SIZE 44100
#define CHANNELS 1

void write_wav(char* filename, float* buffer) {
    static const int FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SndfileHandle outfile(filename, SFM_WRITE, FORMAT, CHANNELS, SR);
    if (outfile) {
        outfile.write(&buffer[0], SIZE);
    }

    // reset
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = 0.0f;
    }
}

int main() {
    char filename[50];
    float buffer_l[SIZE];
    float buffer_r[SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer_l[i] = 0.0f;
        buffer_r[i] = 0.0f;
    }

    std::vector<void*> ports;
    ports.push_back(0);
    ports.push_back(buffer_l);
    ports.push_back(buffer_r);

    rogue::SynthData data;
    data.volume = 0.5;
    data.bus_a_level = 0.5;
    data.bus_a_pan = 0.5;

    data.oscs[0].on = true;
    data.oscs[0].type = 0;
    data.oscs[0].start = 0.0;
    data.oscs[0].width = 0.5;
    data.oscs[0].ratio = 1.0;
    data.oscs[0].level = 1.0;
    data.oscs[0].level_a = 1.0;

    data.envs[0].on = true;
    data.envs[0].pre_delay = 0.0f;
    data.envs[0].attack = 0.1 * SR;
    data.envs[0].hold = 0.5 * SR;
    data.envs[0].decay = 0.4 * SR;
    data.envs[0].sustain = 0.8;
    data.envs[0].release = 0.5 * SR;
    data.envs[0].curve = 0.5;

    rogue::rogueVoice voice(SR, &data);
    voice.set_port_buffers(ports);
    voice.on(69, 64);
    voice.render(0, SIZE / 2);
    voice.off(0);
    voice.render(SIZE / 2, SIZE);

    sprintf(filename, "wavs/voice_%i.wav", 0);
    write_wav(filename, buffer_l);

    // LFO based amp modulation
    data.lfos[0].on = true;
    data.lfos[0].type = 0;
    data.lfos[0].inv = false;
    data.lfos[0].freq = 10.0f;
    data.lfos[0].width = 0.5;
    data.lfos[0].humanize = 0.0;

    data.mods[0].src = M_LFO1_UN;
    data.mods[0].target = M_OSC1_AMP;
    data.mods[0].amount = 0.5;

    voice.on(69, 64);
    voice.render(0, SIZE / 2);
    voice.off(0);
    voice.render(SIZE / 2, SIZE);

    sprintf(filename, "wavs/voice_%i.wav", 1);
    write_wav(filename, buffer_l);

    // filtering
    data.oscs[0].type = 1;

    data.mods[0].src = 0;
    data.mods[0].target = 0;

    data.bus_a_level = 0.0;

    data.filters[0].on = true;
    data.filters[0].type = 0;
    data.filters[0].level = 0.5;
    data.filters[1].pan = 0.5;
    data.filters[0].freq = 1000.0;
    data.filters[0].q = 0.5;

    voice.on(69, 64);
    voice.render(0, SIZE / 2);
    voice.off(0);
    voice.render(SIZE / 2, SIZE);

    sprintf(filename, "wavs/voice_%i.wav", 2);
    write_wav(filename, buffer_l);

}
