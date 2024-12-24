#include <chrono>
#include <coroutine>
#include <iostream>
#include <thread>


// A simple awaitable type
struct Sleeper {
    std::chrono::milliseconds duration;

    bool await_ready() const noexcept
    {
        return false; // Always suspend
    }

    void await_suspend(std::coroutine_handle<> handle) const noexcept
    {
        // Run in a separate thread to mimic asynchronous behavior
        std::thread([handle, this]() {
            std::this_thread::sleep_for(duration);
            handle.resume(); // Resume coroutine after sleeping
        }).detach();
    }

    void await_resume() const noexcept
    {
        // Nothing to return
    }
};

// A task that represents an asynchronous operation
struct Task {
    struct promise_type {
        Task get_return_object()
        {
            return Task(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void               return_void() noexcept {}
        void               unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    Task(std::coroutine_handle<promise_type> h)
        : handle(h) {}
    ~Task()
    {
        if (handle)
            handle.destroy();
    }
};

// A coroutine function using `Sleeper`
Task asyncPrint()
{
    std::cout << "Task started, waiting for 2 seconds...\n";
    co_await Sleeper{std::chrono::seconds(2)}; // Simulate asynchronous delay
    std::cout << "Task resumed after 2 seconds.\n";
}

int main()
{
    asyncPrint();
    std::this_thread::sleep_for(std::chrono::seconds(3)); // Wait for coroutine to complete
    std::cout << "Main thread exiting.\n";
    return 0;
}
