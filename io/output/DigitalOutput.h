#ifndef DIGITALOUTPUT_H_
#define DIGITALOUTPUT_H_

#include "DigitalIO.h"

class DigitalOutput: public DigitalIO {
public:
	DigitalOutput(int const pin, bool const reversed);
	virtual ~DigitalOutput() {
	}
	void setState(bool const newState);

	virtual bool const getState() const {
		return lastState;
	}

	virtual void on() {
		setState(true);
	}

	virtual void off() {
		setState(false);
	}

	void toggle() {
		setState(!lastState);
	}

	virtual void update(unsigned long currentTime) {
		// Do nothings, some output like Leds will override if necessary
	}

private:
	bool lastState;
};

#endif /* DIGITALOUTPUT_H_ */
