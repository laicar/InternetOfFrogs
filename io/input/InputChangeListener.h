#ifndef INPUT_CHANGE_LISTENER_H_
#define INPUT_CHANGE_LISTENER_H_

template <typename StateType>
class InputChangeListener {
public:
	virtual void operator()(StateType const oldState, StateType const newState) = 0;
	virtual ~InputChangeListener() {
	}
};

class DigitalInputChangeListener: public InputChangeListener<bool> {};

class FloatInputChangeListener: public InputChangeListener<float> {};


template <typename StateType>
struct InputChangeListenerFunctionAdapter: public InputChangeListener<StateType> {
	typedef void (*InputChangeListenerFunctionPointer)(StateType const oldState, StateType const newState);

	InputChangeListenerFunctionAdapter(InputChangeListenerFunctionPointer listener):listener(listener) {

	}

	virtual void operator()(StateType const oldState, StateType const newState) {
		listener(oldState, newState);
	}

	virtual ~InputChangeListenerFunctionAdapter() {

	}

	InputChangeListenerFunctionPointer listener;
};

#endif /* INPUT_CHANGE_LISTENER_H_ */
