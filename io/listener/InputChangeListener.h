#ifndef INPUT_CHANGE_LISTENER_H_
#define INPUT_CHANGE_LISTENER_H_

template<typename StateType>
class InputChangeListener {
public:
	virtual void operator()(StateType const & oldState,
			StateType const & newState) = 0;
	virtual ~InputChangeListener() {
	}
};

typedef InputChangeListener<bool> DigitalInputChangeListener;

typedef InputChangeListener<float> FloatInputChangeListener;

template<typename StateType>
struct InputChangeListenerFunctionAdapter: public InputChangeListener<StateType> {

	typedef void (*InputChangeListenerFunctionPointer)(StateType const oldState,
			StateType const newState);

	InputChangeListenerFunctionPointer listener;

	InputChangeListenerFunctionAdapter(
			InputChangeListenerFunctionPointer listener) :
			listener(listener) {
	}

	virtual ~InputChangeListenerFunctionAdapter() {
	}

	virtual void operator()(StateType const & oldState,
			StateType const & newState) {
		listener(oldState, newState);
	}
};

#endif /* INPUT_CHANGE_LISTENER_H_ */
