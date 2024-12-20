#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread/task.hpp>
#include <thread>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(unsigned poolSize) : poolSize(poolSize) {
        for (unsigned i = 0; i < poolSize; ++i) {
            threads.emplace_back(&ThreadPool::run, this);
        }
    }

    ~ThreadPool() { stop(); }

    template <typename F, typename... Args>
    std::shared_ptr<Task> addTask(F&& f, Args&&... args) {
        auto task = createTask(std::forward<F>(f), std::forward<Args>(args)...);

        {
            std::lock_guard<std::mutex> lock(tasksMutex);
            tasks.push(task);
        }

        taskAddCondition.notify_one();
        return task;
    }

    /// @brief Wait for all tasks to complete.
    void waitAll();
    /// @brief Join all threads.
    void stop();

private:
    /// @brief Thread running function.
    void run();

    const unsigned poolSize;
    std::vector<std::thread> threads;

    std::queue<std::shared_ptr<Task>> tasks;
    std::mutex tasksMutex;
    std::condition_variable taskAddCondition;
    std::condition_variable taskCompleteCondition;

    std::atomic<bool> isStopped{false};
    std::atomic<unsigned> activeTasks{0};
};