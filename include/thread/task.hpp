#pragma once

#include <any>
#include <atomic>
#include <functional>
#include <memory>

class Task {
public:
    using TaskFunction = std::function<std::any()>;

    Task(TaskFunction&& func) : func(std::move(func)) {}
    Task(const Task& other) : func(other.func), isReady(other.isReady.load()){};

    template <typename T>
    T getResult() const {
        return std::any_cast<T>(result);
    }

    bool hasResult() const { return isReady; }

    void waitResult() const { isReady.wait(false); }

private:
    friend class ThreadPool;

    TaskFunction func;
    std::any result;
    std::atomic<bool> isReady{false};

    void run() {
        result = func();
        isReady = true;
    }
};

template <typename F, typename... Args>
std::shared_ptr<Task> createTask(F&& f, Args&&... args) {
    auto binded = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    Task::TaskFunction taskFunc;
    if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>) {
        // Void function
        taskFunc = [binded = std::move(binded)]() -> std::any {
            binded();
            return std::any();
        };
    } else {
        // Non-void function
        taskFunc = [binded = std::move(binded)]() -> std::any {
            return binded();
        };
    }

    return make_shared<Task>(std::move(taskFunc));
}
