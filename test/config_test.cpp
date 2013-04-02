/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#include <stdio.h>
#include "test.h"

#include "config.h"

using namespace rogue;

int main() {
    SynthData data;

    facts("config") {
        assert (data.lfos[0].on == false);

        data.lfos[0] = LFOData();
        assert (data.lfos[0].on == false);
    }

}
