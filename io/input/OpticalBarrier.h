#ifndef OPTICAL_BARRIER_H_
#define OPTICAL_BARRIER_H_

#include "DigitalInput.h"


class OpticalBarrier: public DigitalInput {
public:
	OpticalBarrier(int const pin, unsigned int debounceDelay =
			DigitalInput::DEFAULT_DEBOUNCE_DELAY);
	virtual ~OpticalBarrier();
};

#endif /* OPTICAL_BARRIER_H_ */
