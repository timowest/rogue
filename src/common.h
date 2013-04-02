
/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_COMMON_H
#define ROGUE_COMMON_H

#define NOUTS    2       //number of outputs
#define NVOICES 32       //max polyphony
#define SUSTAIN 128
#define SILENCE 0.0001f  //voice choking
#define BUFFER_SIZE 64

#define NOSC    4
#define NDCF    2
#define NLFO    3
#define NENV    5

enum {
  M_ON,
  M_MOD,
  M_PRESSURE,
  M_KEY,
  M_VEL,
  M_LFO0_BI,
  M_LFO0_UN,
  M_LFO1_BI,
  M_LFO1_UN,
  M_LFO2_BI,
  M_LFO2_UN,
  M_LFO3_BI,
  M_LFO3_UN,
  M_EG1,
  M_EG2,
  M_EG3,
  M_EG4,
  M_EG5
};

#endif
