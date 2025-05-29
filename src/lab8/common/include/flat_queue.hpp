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

	T back() const {
		return _data[_tail];
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

	T* find_if(auto&& cond) {
		for (size_t i = 0; i < _size; ++i) {
			size_t index = (_head + i) % N;
			if (cond(_data[index])) {
				return &_data[index];
			}
		}
		return nullptr;
	}

	bool erase(const T& item) {
		for (size_t i = 0; i < _size; ++i) {
			auto t = front();
			pop();
			if (t == item) return true;
			push(t);
		}
		return false;
	}

	struct iterator {
		T* ptr;
		size_t index;
		size_t size;

		iterator(T* p, size_t idx, size_t sz)
			: ptr(p), index(idx), size(sz) {}

		bool operator!=(const iterator& other) const {
			return index != other.index || ptr != other.ptr;
		}

		T& operator*() {
			return ptr[index];
		}

		void operator++() {
			index = (index + 1) % size;
		}
	};

	iterator begin() {
		return iterator(_data, _head, N);
	}

	iterator end() {
		return iterator(_data, _tail, N);
	}
};