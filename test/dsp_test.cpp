/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdio.h>
#include "test.h"

#include "envelope.cpp"
#include "filter.cpp"
#include "lfo.cpp"
#include "oscillator.cpp"

using namespace dsp;

int main() {

    facts("envelope") {
        ADSR adsr;
        adsr.setADSR(10.0f, 5.0f, 1.0f, 5.0f);

        adsr.on();
        assert_eq (0.1f, adsr.tick());
        assert_eq (1.0f, adsr.tick(9));
        assert_eq (1.0f, adsr.tick(10));

        adsr.off();
        assert_eq (0.8f, adsr.tick());
        assert_eq (0.4f, adsr.tick(2));

        assert_eq(0.0f,  adsr.tick(10));
    }

    facts("filter") {
        // TODO
    }

    facts("lfo") {

    }

    facts("oscillator") {

    }

}
