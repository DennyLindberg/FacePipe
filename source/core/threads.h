#pragma once
#include <thread>
#include <mutex>
#include <queue>

template<typename T>
class ThreadSafeQueue
{
public:
	bool Pop(T& Element)
	{
		std::lock_guard<std::mutex> guard(Mutex);

		if (!Queue.empty())
		{
			Element = std::move(Queue.front());
			Queue.pop();
			return true;
		}

		return false;
	}

	void Push(const T& Element)
	{
		std::lock_guard<std::mutex> guard(Mutex);
		Queue.push(Element);
	}

protected:
	std::mutex Mutex;
	std::queue<T> Queue;
};

namespace Threads
{
	unsigned int Count();
	void Join();
}

struct ThreadInfo
{
	unsigned int id = 0;
	bool isDone = false;
};
