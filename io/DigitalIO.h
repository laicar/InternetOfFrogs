#ifndef DIGITALIO_H_
#define DIGITALIO_H_

#include <Arduino.h>
#include "Updatable.h"

class DigitalIO : public Updatable {
public:
	DigitalIO(int const pin, bool const reversed = false);
	virtual ~DigitalIO() {
	}

	virtual bool const getState() const = 0;

	int const getPin() const {
		return pin;
	}

	bool const getReversed() const {
		return reversed;
	}
private:
	int const pin;
	bool const reversed;
};

#endif /* DIGITALIO_H_ */
