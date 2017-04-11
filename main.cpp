/**
 * File:   main.cpp
 * Author: btacklind
 *
 * Created on Nov 28, 2014, 2:02 PM
 */

#include <util/delay.h>
#include <avr/wdt.h>
#include "Board.h"

#include "ThreePhaseController.h"
#include "MLX90363.h"
#include "FilebotInterface/TwillBotInterface.h"
// #include "Debug.h"
#include "Clock.h"
#include "Interpreter.h"
#include "ServoController.h"

using namespace AVR;
using namespace ThreePhaseControllerNamespace;

/**
 * All the init functions should go in here.
 * gcc will automatically call it for us because of the constructor attribute.
 */
void init() __attribute__((constructor));

u1 resetCause = MCUSR;

FileBotInterface::TwillBotInterface<Config::i2cSlaveAddress, Config::i2cBufferIncomingSize, Config::i2cBufferOutgoingSize, Config::i2cBufferIncomingBlocks, 3> fileBotInterface;

void init() {
	// Watch Dog Timer
	wdt_reset();
	wdt_disable();


	// Debug::init();

  // Clear the MCU Status Register.  Indicates previous reset's source.
	MCUSR = 0;

	// Set Enable Interupts.
	sei();

	// Use the Clock that is outside the AVR++ namespace.
	::Clock::init();

	// i2c interface init
	fileBotInterface.init();

	// Interpret i2c communication interface.
	Interpreter::Init();

	// Init for hardware interface.
	ServoController::init();

	// Turn off led.
	Board::LED.output();
	Board::LED.off();

	// End of init
}

/**
 *
 */
void main() {
	// We don't need to call init() because gcc calls it for us before main() even starts.
  //init();

	u2 pos = 0;

	Clock::MicroTime t(0);
	Clock::MicroTime delta = 25_ms;
	Clock::MicroTime now;


	//set to drive at 0 speed
	ThreePhaseController::setAmplitude(0);

	//main loop
	while(1){

		//update hardware
		ServoController::update();

		//get any incoming communications
		u1 const * const buff = fileBotInterface.getIncomingReadBuffer();

		//if there is a communication interpret it
		if (buff) {
			//interpret the new communication
			Interpreter::interpretFromMaster(buff);

			//prepare for next communication
			fileBotInterface.reserveNextReadBuffer();
		}

		//send whatever data we have back to master
		Interpreter::sendNormalDataToMaster();

		//silly fix in case of an error state
		fileBotInterface.fixWriteBuffer();
	}

	//loop in case main loop is disabled
	//allows for interrupts to continue
	while(1);
}
