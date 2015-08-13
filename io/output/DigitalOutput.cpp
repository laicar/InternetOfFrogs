#include "DigitalOutput.h"

DigitalOutput::DigitalOutput(int const pin, bool const reversed) :
		DigitalIO(pin, reversed) {
	pinMode(pin, OUTPUT);
	off();
}

void DigitalOutput::setState(const bool newState) {
	if (newState != lastState) {
		digitalWrite(getPin(), (newState xor getReversed()) ? HIGH : LOW);
		lastState = newState;
	}
}
