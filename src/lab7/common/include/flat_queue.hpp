#pragma once
#include "os_type.hpp"
template <typename T, size_t N>
class FlatQueue {
private:
	T _data[N];
	size_t _head;
	size_t _tail;
	size_t _size;

public:
	FlatQueue()
		: _head(0), _tail(0), _size(0) {}

	void push(const T& item) {
		_data[_tail] = item;
		_tail = (_tail + 1) % N;
		_size++;
	}

	T front() const {
		return _data[_head];
	}

	void pop() {
		_head = (_head + 1) % N;
		_size--;
	}

	size_t size() const {
		return _size;
	}

	bool empty() const {
		return _size == 0;
	}
};