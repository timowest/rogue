#include "wavutils.h"

void delay_test() {
    dsp::Delay delay;
    dsp::DelayL delayl;
    dsp::DelayA delaya;

    delay.setDelay(1000);
    delayl.setDelay(1000);
    delaya.setDelay(1000);

    for (int i = 0; i < SIZE; i++) {
        delay.process(1.0);
        delayl.process(1.0);
        delaya.process(1.0);
    }

    delay.setMax(8192);
    delayl.setMax(8192);
    delaya.setMax(8192);

    for (int i = 0; i < SIZE; i++) {
        delay.process(1.0);
        delayl.process(1.0);
        delaya.process(1.0);
    }
}
