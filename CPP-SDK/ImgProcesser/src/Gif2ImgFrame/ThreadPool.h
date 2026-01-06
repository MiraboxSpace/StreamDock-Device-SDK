#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

class ThreadPool {
public:
	explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
		: stop(false)
	{
		for (size_t i = 0; i < numThreads; ++i)
			workers.emplace_back([this] {
			while (true) {
				std::function<void()> task;

				{   // scoped lock
					std::unique_lock<std::mutex> lock(queueMutex);
					condition.wait(lock, [this] { return stop || !tasks.empty(); });

					if (stop && tasks.empty())
						return;

					task = std::move(tasks.front());
					tasks.pop();
				}

				task(); // Execute task
			}
				});
	}

	// Disable copying
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	// Submit task
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args)
		-> std::future<typename std::invoke_result_t<F, Args...>>
	{
		using return_type = typename std::invoke_result_t<F, Args...>;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

		std::future<return_type> res = task->get_future();

		{   // Thread-safely add to the task queue
			std::unique_lock<std::mutex> lock(queueMutex);
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");
			tasks.emplace([task]() { (*task)(); });
		}

		condition.notify_one();
		return res;
	}

	~ThreadPool()
	{
		{   // Notify all threads to exit
			std::unique_lock<std::mutex> lock(queueMutex);
			stop = true;
		}

		condition.notify_all();
		for (std::thread& worker : workers)
			worker.join();
	}

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex queueMutex;
	std::condition_variable condition;
	std::atomic<bool> stop;
};
