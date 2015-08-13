/*
 * Vector.h
 *
 *  Created on: 30 oct. 2014
 *      Author: sebastien
 */

#ifndef VECTOR_H_
#define VECTOR_H_
#include <Arduino.h>

template<typename Data>
class Vector {
	size_t _size; // Stores no. of actually stored objects
	size_t _capacity; // Stores allocated capacity
	Data *_data; // Stores data
public:
	Vector() :_size(0), _capacity(0), _data(0) {
	}

	Vector(size_t capacity) :_size(0), _capacity(capacity), _data(0) {
		resize(capacity);
	}

	Vector(Vector const &other) :
			_size(other._size), _capacity(other._capacity), _data(0) {
		_data = (Data *) malloc(_capacity * sizeof(Data));
		memcpy(_data, other._data, _size * sizeof(Data));
	}

	~Vector() {
		free(_data);
	}

	Vector &operator=(Vector const &other) {
		free(_data);
		_size = other._size;
		_capacity = other._capacity;
		_data = (Data *) malloc(_capacity * sizeof(Data));
		memcpy(_data, other._data, _size * sizeof(Data));
		return *this;
	}

	void push_back(Data const &x) {
		if (_capacity == _size)
			resize();
		_data[_size++] = x;
	}

	size_t size() const {
		return _size;
	}

	Data const &operator[](size_t idx) const {
		return _data[idx];
	}

	Data &operator[](size_t idx) {
		return _data[idx];
	}

private:
	void resize() {
		_capacity = _capacity ? _capacity * 2 : 1;
	}

	void resize(size_t capacity){
		Data* newdata = (Data*) malloc(capacity * sizeof(Data));
		memcpy(newdata, _data, _size * sizeof(Data));
		free(_data);
		_data = newdata;
	}
};

#endif /* VECTOR_H_ */
