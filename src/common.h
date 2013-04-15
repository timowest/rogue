
/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef ROGUE_COMMON_H
#define ROGUE_COMMON_H

#define NOUTS    2       // number of outputs
#define NVOICES 32       // max polyphony
#define SILENCE 0.0001f  // voice choking
#define BUFFER_SIZE 64

// number of elements
#define NOSC    4
#define NDCF    2
#define NLFO    3
#define NENV    5
#define NMOD    20

// offsets for port access
#define OSC_OFF 14
#define DCF_OFF 10
#define LFO_OFF 7
#define ENV_OFF 8
#define MOD_OFF 3

// modulation sources
enum {
  M_NO_SOURCE,
  M_MOD,
  M_PRESSURE,
  M_KEY, // key / 127.0
  M_VEL, // vel / 127.0
  // lfo
  M_LFO0_BI,
  M_LFO0_UN,
  M_LFO1_BI,
  M_LFO1_UN,
  M_LFO2_BI,
  M_LFO2_UN,
  M_LFO3_BI,
  M_LFO3_UN,
  // env
  M_ENV1,
  M_ENV2,
  M_ENV3,
  M_ENV4,
  M_ENV5,
  M_SIZE
};

// modulation target
enum {
  M_NO_TARGET,
  // osc
  M_OSC1_P, M_OSC1_MOD, M_OSC1_AMP,
  M_OSC2_P, M_OSC2_MOD, M_OSC2_AMP,
  M_OSC3_P, M_OSC3_MOD, M_OSC3_AMP,
  M_OSC4_P, M_OSC4_MOD, M_OSC4_AMP,
  // dcf
  M_DCF1_F, M_DCF1_Q, M_DCF1_PAN, M_DCF1_AMP,
  M_DCF2_F, M_DCF2_Q, M_DCF2_PAN, M_DCF2_AMP,
  // lfo
  M_LFO1_S, M_LFO1_AMP,
  M_LFO2_S, M_LFO2_AMP,
  M_LFO3_S, M_LFO3_AMP,
  // env
  M_ENV1_S, M_ENV1_AMP,
  M_ENV2_S, M_ENV2_AMP,
  M_ENV3_S, M_ENV3_AMP,
  M_ENV4_S, M_ENV4_AMP,
  M_ENV5_S, M_ENV5_AMP,
  // bus
  M_BUSA_PAN, M_BUSB_PAN,
  M_TARGET_SIZE
};

#endif
