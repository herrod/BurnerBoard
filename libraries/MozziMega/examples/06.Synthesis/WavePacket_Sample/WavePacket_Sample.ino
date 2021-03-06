/*  Example of Wavepacket synthesis, using Mozzi sonification library.
 *  This sketch draws on Miller Puckette's 
 *  Pure Data example, F14.wave.packet.pd, with
 *  two overlapping streams of wave packets.
 *  
 *  Demonstrates the WavePacketSample object, which enables a
 *  custom sound table to be used as the audio source for wavepackets.
 *
 *  Circuit: Audio output on digital pin 9 (for STANDARD mode on a Uno or similar), or 
 *  check the README or http://sensorium.github.com/Mozzi/
 *
 *  Mozzi help/discussion/announcements:
 *  https://groups.google.com/forum/#!forum/mozzi-users
 *
 *  Tim Barrass 2013.
 *  This example code is in the public domain.
 */

#include <mozzi_analog.h>
#include <WavePacketSample.h>
#include <RecentAverage.h>
#include <samples/raven_arh_int8.h>

#define FUNDAMENTAL_PIN 0
#define BANDWIDTH_PIN 1
#define CENTREFREQ_PIN 2

// for smoothing the control signals
// use: RecentAverage <number_type, how_many_to_average> myThing
RecentAverage <int, 32> kAverageF;
RecentAverage <int, 32> kAverageBw;
RecentAverage <int, 32> kAverageCf;

WavePacketSample <DOUBLE> wavey; // DOUBLE selects 2 overlapping streams


void setup(){
  wavey.setTable(RAVEN_ARH_DATA);
  pinMode(11,OUTPUT);
  digitalWrite(11,LOW);
  startMozzi();
  adcEnableInterrupt();
}


void updateControl(){
  int f = kAverageF.next(adcGetResult(FUNDAMENTAL_PIN))+1;
  int b = kAverageBw.next(adcGetResult(BANDWIDTH_PIN));
  int cf = kAverageCf.next(2*adcGetResult(CENTREFREQ_PIN));
  wavey.set(f, b, cf);
  adcReadAllChannels();
}


int updateAudio(){
  return wavey.next()>>8; // >>8 for AUDIO_MODE STANDARD
}


void loop(){
  audioHook(); // required here
}

