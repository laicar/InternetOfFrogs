#ifndef DIGITALINPUT_H_
#define DIGITALINPUT_H_

#include "Observable.h"
#include "DigitalIO.h"
#include "InputChangeListener.h"

class DigitalInput: public DigitalIO, public Observable<DigitalInputChangeListener> {
public:
	static const int DEFAULT_DEBOUNCE_DELAY = 50;

	DigitalInput(int const pin, bool const reversed = false,
			unsigned int debounceDelay = DEFAULT_DEBOUNCE_DELAY);
	virtual ~DigitalInput();
	virtual void update(unsigned long currentTime);
	virtual bool const getState() const {
		return (getReversed() xor digitalRead(getPin()));
	}


private:
	unsigned long lastChangeTime;
	bool lastState;
	bool currentState;
	unsigned int debounceDelay;
};

#endif /* DIGITALINPUT_H_ */
