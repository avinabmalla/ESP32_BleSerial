// Thread safe ring buffer
#pragma once
#include <Arduino.h>

template <size_t N>
class ByteRingBuffer
{
private:
	uint8_t buffer[N];
	int head = 0;
	int tail = 0;

public:
	void add(uint8_t value)
	{
		buffer[head] = value;
		head = (head + 1) % N;
		if (head == tail)
		{
			tail = (tail + 1) % N;
		}
	}
	uint8_t pop()
	{
		// pops the oldest value off the ring buffer
		if (head == tail)
		{
			return -1;
		}
		else
		{
			uint8_t value = buffer[tail];
			tail = (tail + 1) % N;
			return value;
		}
	}

	void clear()
	{
		head = 0;
		tail = 0;
	}

	uint8_t get(size_t index)
	{
		// this.get(0) is the oldest value, this.get(this.getLength() - 1) is the newest value
		if (index >= this->getLength())
		{
			return -1;
		}
		else
		{
			return buffer[(tail + index) % N];
		}
	}

	size_t getLength()
	{
		if (head >= tail)
		{
			return head - tail;
		}
		else
		{
			return N - tail + head;
		}
	}

};
