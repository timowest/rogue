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

#include "wavutils.h"
#include "oscillator_test.h"
#include "filter_test.h"
#include "envelope_test.h"
#include "lfo_test.h"

int main() {
    // ?!?
    dsp::DCBlocker dcBlocker;
    dcBlocker.setSamplerate(SR);

    oscillator_test();
    filter_test();
    envelope_test();
    lfo_test();
    return 0;
}
