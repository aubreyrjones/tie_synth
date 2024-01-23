#pragma once
#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
      

extern AudioInputI2S i2s1;
extern AudioSynthWaveformSine sine1;
extern AudioOutputI2S i2s2;
extern AudioConnection patchCord1;
extern AudioConnection patchCord2;
extern AudioControlSGTL5000 sgtl5000_1;
