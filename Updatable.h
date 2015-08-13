/*
 * Updatable.h
 *
 *  Created on: 30 oct. 2014
 *      Author: sebastien
 */

#ifndef UPDATABLE_H_
#define UPDATABLE_H_

class Updatable {
public:
	virtual ~Updatable();
	virtual void update(unsigned long currentTime) = 0;
};

#endif /* UPDATABLE_H_ */
