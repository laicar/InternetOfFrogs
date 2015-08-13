#include "DigitalInput.h"

DigitalInput::DigitalInput(int const pin, bool const reversed,
		unsigned int debounceDelay) :
		DigitalIO(pin, reversed), debounceDelay(debounceDelay) {
	pinMode(pin, INPUT);
	lastChangeTime = millis();
	currentState = getState();
	lastState = currentState;
}

void DigitalInput::update(unsigned long currentTime) {
	// Get current state
	bool newState = getState();

	// If the switch changed, due to noise or pressing:
	if (newState != lastState) {
		// reset the debouncing timer
		lastChangeTime = currentTime;
	}

	if ((newState != currentState)
			&& ((currentTime - lastChangeTime) > debounceDelay)) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		bool oldState = currentState;
		currentState = newState;
		notify(oldState, newState);
	}

	// Save last state
	lastState = newState;
}

DigitalInput::~DigitalInput() {
}
