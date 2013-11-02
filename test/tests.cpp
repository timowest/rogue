#include <sndfile.hh>

#include "delay.cpp"
#include "effects.cpp"
#include "envelope.cpp"
#include "filter.cpp"
#include "lfo.cpp"
#include "oscillator.cpp"
#include "tables.cpp"

#include <iostream>

static void error(const char* text) {
    std::cout << text << std::endl;
}

template<class T>
static void error(const char* text, T val) {
    static char msg[100];
    sprintf(msg, text, val);
    std::cout << msg << std::endl;
}

template<class T, class U>
static void error(const char* text, T val, U val2) {
    static char msg[100];
    sprintf(msg, text, val, val2);
    std::cout << msg << std::endl;
}

#include "wavutils.h"
#include "effects_test.h"
#include "envelope_test.h"
#include "filter_test.h"
#include "lfo_test.h"
#include "oscillator_test.h"

int main() {
    // ?!?
    dsp::DCBlocker dcBlocker;
    dcBlocker.setSamplerate(SR);

    effects_test();
    envelope_test();
    filter_test();
    lfo_test();
    oscillator_test();

    return 0;
}
