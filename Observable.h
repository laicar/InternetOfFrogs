/*
 * Observable.h
 *
 *  Created on: 30 oct. 2014
 *      Author: sebastien
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "Vector.h"
#include "InputChangeListener.h"

#define LISTENERS_DEFAULT_NUMBER 5

template<typename Listener, typename StateType = bool>
class Observable{
public:
	Observable():listeners(LISTENERS_DEFAULT_NUMBER){}

	virtual ~Observable(){}

	void attach(Listener* listener) {
		listeners.push_back(listener);
	}

	void notify(StateType const oldState, StateType const newState){
		// Ping listeners
		for (size_t i = 0; i < listeners.size(); ++i) {
			Listener* listenerCour = listeners[i];
			if (listenerCour) {
				(*listenerCour)(oldState, newState);
			}
		}
	}

private:
	Vector<Listener*> listeners;
};

#endif /* OBSERVABLE_H_ */
