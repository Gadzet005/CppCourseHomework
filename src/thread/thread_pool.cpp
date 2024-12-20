#include <thread/thread_pool.hpp>

void ThreadPool::stop() {
    if (isStopped) {
        return;
    }

    isStopped = true;
    taskAddCondition.notify_all();
    for (auto& thread : threads) {
        thread.join();
    }
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(tasksMutex);
    taskCompleteCondition.wait(
        lock, [this]() { return activeTasks == 0 && tasks.empty(); });
}

void ThreadPool::run() {
    while (true) {
        std::unique_lock<std::mutex> lock(tasksMutex);
        taskAddCondition.wait(lock,
                              [this]() { return !tasks.empty() || isStopped; });
        if (isStopped) {
            return;
        }

        auto task = std::move(tasks.front());
        tasks.pop();
        activeTasks++;
        lock.unlock();

        task->run();

        lock.lock();
        activeTasks--;
        taskCompleteCondition.notify_one();
    }
}
