rogue
=====

multimode softsynth with LV2 interface

Features
* 4 oscillators with 
 * Virtual Analog, Phase Distortion, FM and other waveforms
 * PWM, PM, Sync, RM and AM modulation
* 2 filters with Biquad, Moog and SVF style modes
* 4 AHDSR envelopes with customizable curve
* 4 LFOs
* 20 Modulation matrix slots
* Effects: Chorus, Phaser, Delay and Reverb
* Qt4 based UI

Dependencies
* LV2 http://lv2plug.in/
* lvtk http://lvtoolkit.org/
* fftw3 http://www.fftw.org/
* Qt4 http://qt-project.org/

Build tools
* make
* gcc (>= 4.7)
* python with python-rdflib

Installation

    make
    sudo make install    
    make run ; if you have jalv installed
