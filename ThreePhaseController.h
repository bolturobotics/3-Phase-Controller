/* 
 * File:   ThreePhaseController.h
 * Author: Cameron
 *
 * Created on October 22, 2015, 2:21 AM
 * 
 * Takes in an amplitude to be commanded 
 * handles input from predictor
 * outputs to the driver (hardware)
 */

#ifndef THREEPHASECONTROLLER_H
#define THREEPHASECONTROLLER_H

#include <AVR++/basicTypes.h>

#include "ThreePhaseDriver.h"
#include <avr/interrupt.h>
#include "ThreePhasePositionEstimator.h"

ISR(TIMER4_OVF_vect);

namespace ThreePhaseControllerNamespace {

  using namespace AVR;

  /**
   * This static class wraps around the ThreePhaseDriver and some position estimation
   * to continuously tell the Driver what phase to drive at for some constant amplitude drive.
   *
   * This presents the minimal interface that all "motors" should have:
   *  - Amplitude control of PWM
   *  - Position (and velocity) feedback estimations
   *
   * "Servos" for position, velocity, current, power, whatever are outside the scope
   * of this class. See ServoController for more.
   */
  class ThreePhaseController {
    /**
     * Called periodically (nominally 31.25kHz) by TIMER 4 ISR
     */
    static inline void controlLoop() __attribute__((hot));
    friend void ::TIMER4_OVF_vect();

    static u1 magRoll;
    static u2 roll;

    volatile static bool enabled;

    /**
     * Number of cycles the PWM timer makes per measurement ready from MLX. We pick
     * a number such that we wait at least 1ms between measurements, otherwise the
     * data won't be ready.
     *
     * = frequency(PWM) * period(MLX) = 32kHz * 1.25ms = 40;
     */
    static constexpr u1 cyclesPWMPerMLX = 40;

    //are we trying to go forward
    static bool isForwardTorque;

    /**
     * Redundant variable for if amplitude == 0
     */
    static bool isZeroTorque;

    /**
     * 90 degree phase shift
     */
    static constexpr u2 output90DegPhaseShift = ThreePhaseDriver::StepsPerCycle / 4;

  public:
    /**
     * Initialize the controller
     */
    static void init();

    /**
     * Enable the controller
     */
    inline static void enable() {
      enabled = true;
    }

    /**
     * Disable the controller
     */
    inline static void disable() {
      enabled = false;
    }

    /**
     * Abstraction around possible amplitude values.
     * 
     * Amplitude is an 8-bit number + a direction.
     * 
     * We also store zero for speed. (Maybe this should not be done)
     */
    class Amplitude {
      bool forward;
      bool zero;
      u1 amplitude;
      friend class ThreePhaseController;

    public:

      inline Amplitude(s2 const t) : forward(t >= 0), zero(t == 0), amplitude(forward ? t : -t) {
      };

      inline Amplitude(const bool fwd, u1 const ampl) : forward(fwd), zero(ampl == 0), amplitude(ampl) {
      };
    };

    /**
     * Set the desired drive amplitude
     */
    static void setAmplitude(const Amplitude t);

    /**
     * Get the current amplitude
     *
     * @return s2 (range [-255, 255])
     */
    static inline s2 getAmplitude() {
      return isForwardTorque ? ThreePhaseDriver::getAmplitude() : -(s2) (ThreePhaseDriver::getAmplitude());
    };

    //get the last measured angular position (by the magnetometer & converted to phase units)

    inline static u2 getMeasuredPosition() {
      return ThreePhasePositionEstimator::getMeasuredPosition();
    }

  };

};

#endif /* THREEPHASECONTROLLER_H */
