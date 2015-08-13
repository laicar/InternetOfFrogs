#include "OpticalBarrier.h"

OpticalBarrier::OpticalBarrier(int const pin, unsigned int debounceDelay) :
		DigitalInput(pin, false, debounceDelay) {
}

OpticalBarrier::~OpticalBarrier() {
}
