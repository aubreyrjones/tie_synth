#pragma once
#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
      

extern AudioInputI2S analog_in;
extern AudioMixer4 mixer4;
extern AudioSynthWaveform va_osc1;
extern AudioSynthWaveform va_osc2;
extern AudioSynthWaveformModulated va_osc3;
extern AudioMixer4 va_osc_mixer;
extern AudioEffectWaveshaper va_waveshape;
extern AudioSynthWaveformDc va_wavefolder_control;
extern AudioEffectWaveFolder va_wavefolder;
extern AudioSynthWaveformDc va_filter_control;
extern AudioFilterStateVariable va_filter;
extern AudioMixer4 va_filter_mixer;
extern AudioAmplifier va_breath_amp;
extern AudioMixer4 output_mixer;
extern AudioAmplifier output_amp;
extern AudioOutputI2S analog_out;
extern AudioConnection patchCord1;
extern AudioConnection patchCord2;
extern AudioConnection patchCord3;
extern AudioConnection patchCord4;
extern AudioConnection patchCord5;
extern AudioConnection patchCord6;
extern AudioConnection patchCord7;
extern AudioConnection patchCord8;
extern AudioConnection patchCord9;
extern AudioConnection patchCord10;
extern AudioConnection patchCord11;
extern AudioConnection patchCord12;
extern AudioConnection patchCord13;
extern AudioConnection patchCord14;
extern AudioConnection patchCord15;
extern AudioConnection patchCord16;
extern AudioConnection patchCord17;
extern AudioConnection patchCord18;
extern AudioConnection patchCord19;
extern AudioControlSGTL5000 sgtl5000_1;
