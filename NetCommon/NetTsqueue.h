#ifndef __NETTSQUEUE_H__
#define __NETTSQUEUE_H__
#include "NetCommon.h"

namespace netCommon
{
	template<typename T>
	class Ts_queue
	{
	public:
		Ts_queue() = default;
		Ts_queue(const Ts_queue<T>&) = delete;
		virtual ~Ts_queue() { clear(); }
	public:
		const T& front()
		{
			std::scoped_lock<std::mutex> lock(queueMutex);
			//std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.front();
		}

		const T& front()const
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.front();
		}

		const T& back()
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.back();
		}

		const T& back()const
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.back();
		}

		void emplace_back(const T& item)
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			deqQueue.emplace_back(std::move(item));
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.notify_one();
		}
		void emplace_front(const T& item)
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			deqQueue.emplace_front(std::move(item));
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.notify_one();
		}

		decltype(auto) count()
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.size();
		}
		decltype(auto) count() const
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.size();
		}

		decltype(auto) empty() 
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.empty();
		}

		decltype(auto) empty()const
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.empty();
		}

		void clear()
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			return deqQueue.clear();
		}

		decltype(auto) pop_back()
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			auto data = std::move(deqQueue.back());
			deqQueue.pop_back();
			return data;
		}
		decltype(auto) pop_front()
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			auto data = std::move(deqQueue.front());
			deqQueue.pop_front();
			return data;
		}

		//template <typename T>
		void swap(std::deque<T>& other)
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			deqQueue.swap(other);
		}

		void wait()
		{
			while (empty())
			{
				std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.wait(ul);
			}
		}

	protected:
		std::condition_variable cvBlocking;
		mutable std::mutex muxBlocking;
		mutable std::mutex queueMutex;
		std::deque<T> deqQueue;
	};

}


#endif

