/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   HallWatcher.cpp
 * Author: Cameron
 *
 * Created on October 12, 2017, 1:53 PM
 */

#include "HallWatcher.hpp"

using namespace AVR;
using namespace ThreePhaseControllerNamespace;

u1 volatile HallWatcher::state = 0b111;
void (*HallWatcher::stateChangeReceiver)() = nullptr;

#ifdef QUANTUM_DRIVE
void INT0_vect() { HallWatcher::checkH1(); }
void INT1_vect() { HallWatcher::checkH2(); }
void PCINT0_vect() { HallWatcher::checkH3(); }

void HallWatcher::init() {
  // set edge detection INT0 & INT1
  EICRA = 0b00000101;
  // Enable INT0 & INT1
  EIMSK = 0b00000011;

  // Enable PCINT4
  PCMSK0 = 0b00010000;
#endif

#if defined(HOVER_DRIVE) || defined(BED_CONTROLLER)
  void INT0_vect() {}
  void INT1_vect() {}
  void PCINT0_vect() {}

  void HallWatcher::init() {
    return;
#endif

    // TODO: Make the above ifdef/endif less hacky

    // Enable pin change interrupts in general
    PCICR = 0b00000001;

    checkH1();
    checkH2();
    checkH3();
  }
