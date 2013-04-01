/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_TEST_H
#define ROGUE_TEST_H

#include <math.h>
#include <assert.h>

#define facts(label) if(true)
#define fact(label)  if(true)

// TODO use a template function for this

void assert_eq(float a, float b) {
    float delta = fabs(a - b);
    assert (delta < 0.0001f);
}

#endif
